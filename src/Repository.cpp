#include "Repository.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

void check(PGconn* conn, PGresult* res, const char* what) {
    const ExecStatusType status = PQresultStatus(res);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
        return;
    }
    std::string err = what;
    err += ": ";
    err += PQerrorMessage(conn);
    PQclear(res);
    throw std::runtime_error(err);
}

}  // namespace

InventoryRepository::InventoryRepository(std::string conninfo) : conninfo_(std::move(conninfo)) {
    conn_ = PQconnectdb(conninfo_.c_str());
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn_);
        PQfinish(conn_);
        conn_ = nullptr;
        throw std::runtime_error("postgres connect failed: " + err);
    }
}

InventoryRepository::~InventoryRepository() {
    if (conn_) {
        PQfinish(conn_);
    }
}

void InventoryRepository::exec(const std::string& sql) {
    PGresult* res = PQexec(conn_, sql.c_str());
    check(conn_, res, "exec");
    PQclear(res);
}

PGresult* InventoryRepository::execParams(const char* sql, int n, const char* const* values) {
    PGresult* res = PQexecParams(conn_, sql, n, nullptr, values, nullptr, nullptr, 0);
    check(conn_, res, sql);
    return res;
}

std::string InventoryRepository::readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot read " + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::int64_t InventoryRepository::asInt64(const char* text) {
    return text ? std::stoll(text) : 0;
}

void InventoryRepository::applySqlFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(mu_);
    exec(readFile(path));
}

std::vector<LocationPick> InventoryRepository::locationsWithStock(const std::string& sku, int qty) {
    const std::string qty_s = std::to_string(qty);
    const char* values[] = {sku.c_str(), qty_s.c_str()};
    std::lock_guard<std::mutex> lock(mu_);
    PGresult* res = execParams(
        "SELECT location_id, location_type FROM stock WHERE sku=$1 AND qty>=$2::int",
        2, values);
    std::vector<LocationPick> out;
    const int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        out.push_back(LocationPick{PQgetvalue(res, i, 0), PQgetvalue(res, i, 1)});
    }
    PQclear(res);
    return out;
}

HoldRecord InventoryRepository::reserve(const std::string& sku, const std::string& location_id, int qty,
                                        int ttl_seconds) {
    const std::string qty_s = std::to_string(qty);
    const std::string ttl_s = std::to_string(ttl_seconds);
    const char* lock_vals[] = {sku.c_str(), location_id.c_str()};
    const char* upd_vals[] = {qty_s.c_str(), sku.c_str(), location_id.c_str()};
    const char* ins_vals[] = {sku.c_str(), location_id.c_str(), qty_s.c_str(), ttl_s.c_str()};

    std::lock_guard<std::mutex> lock(mu_);
    exec("BEGIN");
    try {
        PGresult* locked = execParams(
            "SELECT qty FROM stock WHERE sku=$1 AND location_id=$2 FOR UPDATE",
            2, lock_vals);
        if (PQntuples(locked) == 0) {
            PQclear(locked);
            throw std::runtime_error("unknown stock row");
        }
        const int available = std::stoi(PQgetvalue(locked, 0, 0));
        PQclear(locked);
        if (available < qty) {
            throw std::runtime_error("insufficient stock");
        }

        PGresult* upd = execParams(
            "UPDATE stock SET qty = qty - $1::int WHERE sku=$2 AND location_id=$3",
            3, upd_vals);
        PQclear(upd);

        PGresult* ins = execParams(
            "INSERT INTO holds (sku, location_id, qty, status, expires_at) "
            "VALUES ($1,$2,$3::int,'reserved', NOW() + ($4::int * INTERVAL '1 second')) "
            "RETURNING hold_id",
            4, ins_vals);
        HoldRecord rec;
        rec.hold_id = asInt64(PQgetvalue(ins, 0, 0));
        rec.sku = sku;
        rec.location_id = location_id;
        rec.qty = qty;
        PQclear(ins);
        exec("COMMIT");
        return rec;
    } catch (...) {
        PGresult* rb = PQexec(conn_, "ROLLBACK");
        PQclear(rb);
        throw;
    }
}

bool InventoryRepository::commit(std::int64_t hold_id) {
    const std::string id_s = std::to_string(hold_id);
    const char* id_vals[] = {id_s.c_str()};

    std::lock_guard<std::mutex> lock(mu_);
    exec("BEGIN");
    try {
        PGresult* row = execParams(
            "SELECT sku, qty, status FROM holds WHERE hold_id=$1::bigint FOR UPDATE",
            1, id_vals);
        if (PQntuples(row) == 0 || std::string(PQgetvalue(row, 0, 2)) != "reserved") {
            PQclear(row);
            exec("ROLLBACK");
            return false;
        }
        const std::string sku = PQgetvalue(row, 0, 0);
        const std::string qty = PQgetvalue(row, 0, 1);
        PQclear(row);

        PGresult* upd = execParams(
            "UPDATE holds SET status='committed' WHERE hold_id=$1::bigint", 1, id_vals);
        PQclear(upd);

        const char* ord_vals[] = {id_s.c_str(), sku.c_str(), qty.c_str()};
        PGresult* ord = execParams(
            "INSERT INTO orders (hold_id, sku, qty, state) VALUES ($1::bigint,$2,$3::int,'paid')",
            3, ord_vals);
        PQclear(ord);
        exec("COMMIT");
        return true;
    } catch (...) {
        PGresult* rb = PQexec(conn_, "ROLLBACK");
        PQclear(rb);
        throw;
    }
}

bool InventoryRepository::release(std::int64_t hold_id, const std::string& new_status) {
    const std::string id_s = std::to_string(hold_id);
    const char* id_vals[] = {id_s.c_str()};

    std::lock_guard<std::mutex> lock(mu_);
    exec("BEGIN");
    try {
        PGresult* row = execParams(
            "SELECT sku, location_id, qty, status FROM holds WHERE hold_id=$1::bigint FOR UPDATE",
            1, id_vals);
        if (PQntuples(row) == 0 || std::string(PQgetvalue(row, 0, 3)) != "reserved") {
            PQclear(row);
            exec("ROLLBACK");
            return false;
        }
        const std::string sku = PQgetvalue(row, 0, 0);
        const std::string loc = PQgetvalue(row, 0, 1);
        const std::string qty = PQgetvalue(row, 0, 2);
        PQclear(row);

        const char* st_vals[] = {new_status.c_str(), id_s.c_str()};
        PGresult* upd = execParams(
            "UPDATE holds SET status=$1 WHERE hold_id=$2::bigint", 2, st_vals);
        PQclear(upd);

        const char* stock_vals[] = {qty.c_str(), sku.c_str(), loc.c_str()};
        PGresult* stock = execParams(
            "UPDATE stock SET qty = qty + $1::int WHERE sku=$2 AND location_id=$3",
            3, stock_vals);
        PQclear(stock);

        const char* ord_vals[] = {id_s.c_str(), sku.c_str(), qty.c_str(), new_status.c_str()};
        PGresult* ord = execParams(
            "INSERT INTO orders (hold_id, sku, qty, state) VALUES ($1::bigint,$2,$3::int,$4)",
            4, ord_vals);
        PQclear(ord);
        exec("COMMIT");
        return true;
    } catch (...) {
        PGresult* rb = PQexec(conn_, "ROLLBACK");
        PQclear(rb);
        throw;
    }
}

int InventoryRepository::onHand(const std::string& sku) {
    const char* values[] = {sku.c_str()};
    std::lock_guard<std::mutex> lock(mu_);
    PGresult* res = execParams("SELECT COALESCE(SUM(qty),0) FROM stock WHERE sku=$1", 1, values);
    const char* text = PQgetvalue(res, 0, 0);
    const std::string raw = text ? text : "0";
    PQclear(res);
    return std::stoi(raw);
}

int InventoryRepository::committedQty(const std::string& sku) {
    const char* values[] = {sku.c_str()};
    std::lock_guard<std::mutex> lock(mu_);
    PGresult* res = execParams(
        "SELECT COALESCE(SUM(qty),0) FROM orders WHERE sku=$1 AND state='paid'", 1, values);
    const char* text = PQgetvalue(res, 0, 0);
    const std::string raw = text ? text : "0";
    PQclear(res);
    return std::stoi(raw);
}

#include "ag/ag_persist.hpp"
#include "ag/ag_error.hpp"
#include "ag/ag_expr.hpp"
#include <sstream>

#if AG_ENABLE_SQLITE
#include <sqlite3.h>
#endif

namespace ag {

AlgorithmStore::AlgorithmStore(std::string path) : path_(std::move(path)) {}

AlgorithmStore::~AlgorithmStore() {
#if AG_ENABLE_SQLITE
    if (db_) sqlite3_close(static_cast<sqlite3*>(db_));
#endif
    db_ = nullptr;
}

bool AlgorithmStore::enabled() const {
#if AG_ENABLE_SQLITE
    return true;
#else
    return false;
#endif
}

void AlgorithmStore::open() {
#if AG_ENABLE_SQLITE
    sqlite3* raw = nullptr;
    if (sqlite3_open(path_.c_str(), &raw) != SQLITE_OK) {
        std::string msg = raw ? sqlite3_errmsg(raw) : "unknown sqlite error";
        if (raw) sqlite3_close(raw);
        throw Error("sqlite open failed: " + msg);
    }
    db_ = raw;
    const char* sql =
        "CREATE TABLE IF NOT EXISTS algorithms ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "fitness REAL NOT NULL,"
        "novelty REAL NOT NULL,"
        "accuracy REAL NOT NULL,"
        "stability REAL NOT NULL,"
        "complexity REAL NOT NULL,"
        "expression TEXT NOT NULL,"
        "signature TEXT NOT NULL,"
        "notes TEXT NOT NULL,"
        "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");";
    char* err = nullptr;
    if (sqlite3_exec(raw, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown sqlite schema error";
        sqlite3_free(err);
        throw Error("sqlite schema failed: " + msg);
    }
#else
    throw Error("SQLite persistence disabled at build time");
#endif
}

void AlgorithmStore::save_genome(const Genome& genome, const std::string& expression, const std::string& notes) {
#if AG_ENABLE_SQLITE
    AG_REQUIRE(db_ != nullptr, "database not opened");
    const char* sql =
        "INSERT OR REPLACE INTO algorithms "
        "(id,name,fitness,novelty,accuracy,stability,complexity,expression,signature,notes) "
        "VALUES (?,?,?,?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt = nullptr;
    auto* db = static_cast<sqlite3*>(db_);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw Error("sqlite prepare failed");
    }
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(genome.id & 0x7fffffffffffffffULL));
    sqlite3_bind_text(stmt, 2, genome.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, genome.fitness);
    sqlite3_bind_double(stmt, 4, genome.novelty);
    sqlite3_bind_double(stmt, 5, genome.accuracy);
    sqlite3_bind_double(stmt, 6, genome.stability);
    sqlite3_bind_double(stmt, 7, genome.complexity);
    sqlite3_bind_text(stmt, 8, expression.c_str(), -1, SQLITE_TRANSIENT);
    const std::string sig = genome_signature(genome);
    sqlite3_bind_text(stmt, 9, sig.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, notes.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw Error("sqlite insert failed: " + msg);
    }
    sqlite3_finalize(stmt);
#else
    (void)genome; (void)expression; (void)notes;
    throw Error("SQLite persistence disabled at build time");
#endif
}

std::vector<Genome> AlgorithmStore::load_top(int limit) {
    std::vector<Genome> out;
#if AG_ENABLE_SQLITE
    AG_REQUIRE(db_ != nullptr, "database not opened");
    const char* sql = "SELECT id,name,fitness,novelty,accuracy,stability,complexity FROM algorithms ORDER BY fitness DESC LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;
    auto* db = static_cast<sqlite3*>(db_);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw Error("sqlite prepare failed");
    }
    sqlite3_bind_int(stmt, 1, limit);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Genome g;
        g.id = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        g.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        g.fitness = sqlite3_column_double(stmt, 2);
        g.novelty = sqlite3_column_double(stmt, 3);
        g.accuracy = sqlite3_column_double(stmt, 4);
        g.stability = sqlite3_column_double(stmt, 5);
        g.complexity = sqlite3_column_double(stmt, 6);
        out.push_back(std::move(g));
    }
    sqlite3_finalize(stmt);
#else
    (void)limit;
#endif
    return out;
}

} // namespace ag

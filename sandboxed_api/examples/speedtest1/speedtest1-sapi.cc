#include "sandboxed_api/examples/speedtest1/lib/sandbox.h"
#include "sandboxed_api/examples/speedtest1/lib/speedtest1-lib-sapi.sapi.h"
#include "sandboxed_api/transaction.h"
#include "sandboxed_api/vars.h"
#include "sandboxed_api/util/flag.h"
#include "sandboxed_api/util/canonical_errors.h"
#include "sandboxed_api/util/status.h"
#include "sqlite3.h"
#include "absl/time/clock.h"

extern "C" {
    int speedtest1_main(int argc, char **argv);
}

Speedtest1Api *global_api;

typedef struct sqlite3_sealed sqlite3_sealed;
typedef struct sqlite3_stmt_sealed sqlite3_stmt_sealed;
extern "C" void reset_random_s(void){
    global_api->reset_random_s();
}

static int last_test_num = 0;
static int64_t test_start = 0;
extern "C" void new_sub_test(int iCol){
    int cur_num = last_test_num;
    last_test_num = iCol;
    if (test_start != 0) {
        printf("measurement for test %d: %ldus\n", cur_num, (absl::GetCurrentTimeNanos() - test_start) / 1000);
    }

    test_start = absl::GetCurrentTimeNanos();
}

extern "C" int sqlite3_open_s(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3_sealed **ppDb          /* OUT: SQLite db handle */
){
    CHECK(filename != nullptr);
    sapi::v::LenVal param(filename, strlen(filename) + 1);
    sapi::v::GenericPtr ptr(nullptr);
    auto status = global_api->sqlite3_open_s(param.PtrBefore(), ptr.PtrBoth());
    // printf("ptr: %p\n", reinterpret_cast<sqlite3_sealed *>(ptr.GetValue()));
    CHECK(status.ok());
    *ppDb = reinterpret_cast<sqlite3_sealed *>(ptr.GetValue());
    return status.ValueOrDie();
}

// No callback or errormessage
extern "C" int sqlite3_exec_s(
  sqlite3_sealed** db, /* An open database */
  const char *sql /* SQL to be evaluated */
){
    // printf("exec sql: %s\n", sql);
    sapi::v::LenVal param(sql, strlen(sql) + 1);
    sapi::v::GenericPtr ptr(*db);
    // printf("ptr: %p\n", reinterpret_cast<sqlite3_sealed *>(ptr.GetValue()));
    auto status = global_api->sqlite3_exec_s(ptr.PtrBefore(), param.PtrBefore());
    CHECK(status.ok());
    return status.ValueOrDie();
}

extern "C" int sqlite3_prepare_v2_s(
  sqlite3_sealed **db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt_sealed **ppStmt  /* OUT: Statement handle */
){
    sapi::v::LenVal sql(zSql, strlen(zSql) + 1);
    sapi::v::GenericPtr dbptr(*db);
    sapi::v::GenericPtr stmtptr(nullptr);
    auto status = global_api->sqlite3_prepare_v2_s(dbptr.PtrBefore(), sql.PtrBefore(), nByte, stmtptr.PtrBoth());
    CHECK(status.ok());
    *ppStmt = reinterpret_cast<sqlite3_stmt_sealed *>(stmtptr.GetValue());
    return status.ValueOrDie();
}

extern "C" int sqlite3_step_s(
  sqlite3_stmt_sealed**stmt
){
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_step_s(ptr.PtrBefore());
    CHECK(status.ok());
    return status.ValueOrDie();
}

extern "C" int sqlite3_column_count_s(
  sqlite3_stmt_sealed**stmt
){
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_column_count_s(ptr.PtrBefore());
    CHECK(status.ok());
    return status.ValueOrDie();
}

extern "C" const unsigned char *sqlite3_column_text_s(
    sqlite3_stmt_sealed**stmt,
    int iCol
){
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_column_text_s(ptr.PtrBefore(), iCol);
    CHECK(status.ok());
    // return (const unsigned char*) status.ValueOrDie();
    return (const unsigned char*) "";
}

extern "C" int sqlite3_bind_int_s(sqlite3_stmt_sealed**stmt, int col, int val){
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_bind_int_s(ptr.PtrBefore(), col, val);
    CHECK(status.ok());
    return status.ValueOrDie();
}
extern "C" int sqlite3_bind_int64_s(sqlite3_stmt_sealed**stmt, int col, sqlite3_int64 val){
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_bind_int64_s(ptr.PtrBefore(), col, val);
    CHECK(status.ok());
    return status.ValueOrDie();
}
extern "C" int sqlite3_bind_text_s(sqlite3_stmt_sealed** stmt,int arg1,const char* text,int n){
    // printf("text: %s, n: %d", text, n);
    sapi::v::LenVal sql(text, (n >= 0) ? n : (strlen(text) + 1));
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_bind_text_s(ptr.PtrBefore(), arg1, sql.PtrBefore(), n);
    CHECK(status.ok());
    return status.ValueOrDie();
}

extern "C" int sqlite3_reset_s(
  sqlite3_stmt_sealed**stmt
){
    sapi::v::GenericPtr ptr(*stmt);
    auto status = global_api->sqlite3_reset_s(ptr.PtrBefore());
    CHECK(status.ok());
    return status.ValueOrDie();
}

extern "C" int sqlite3_finalize_s(
  sqlite3_stmt_sealed*stmt
){
    sapi::v::GenericPtr ptr(stmt);
    auto status = global_api->sqlite3_finalize_s(ptr.PtrBefore());
    CHECK(status.ok());
    return status.ValueOrDie();

}
extern "C" int sqlite3_close_s(
  sqlite3_sealed*db /* An open database */
){
    sapi::v::GenericPtr ptr(db);
    auto status = global_api->sqlite3_close_s(ptr.PtrBefore());
    CHECK(status.ok());
    return status.ValueOrDie();
}

namespace {

    class Speedtest1Transaction : public sapi::Transaction {
    public:
        Speedtest1Transaction(std::unique_ptr<sapi::Sandbox> sandbox, int argc, char** argv)
            : sapi::Transaction(std::move(sandbox)), argc_(argc), argv_(argv) {
            sapi::Transaction::SetTimeLimit(kTimeOutVal);
        }

    private:
        int argc_;
        char** argv_;

        // Default timeout value for each transaction run.
        const time_t kTimeOutVal = 10000;

        // The main processing function.
        sapi::Status Main() override;
    };

    sapi::Status Speedtest1Transaction::Main() {
        Speedtest1Api f(GetSandbox());
        global_api = &f;
        global_api->nop();
        global_api->hello_world();

        speedtest1_main(argc_, argv_);
        return sapi::OkStatus();
    }

}  // namespace

int main(int argc, char** argv) {
    sapi::Status status;
    google::InitGoogleLogging(argv[0]);

    Speedtest1Transaction sapi{absl::make_unique<Speedtest1SapiSandbox>(), argc, argv};
    status = sapi.Run();
    LOG(INFO) << "Final run result 2: " << status.message();
    CHECK(status.ok());

    return 0;
}

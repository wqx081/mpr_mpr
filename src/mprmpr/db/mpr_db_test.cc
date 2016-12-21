#include "mprmpr/db/db.h"

const std::string kHostname("localhost");
const std::string kUsername("root");
const std::string kPassword("111111");
const std::string kDatabase("test_db");

void PrintResult(mprmpr::db::Result* result) {
  for (auto row : *result) {
    std::cout << "{" << std::endl;
    for (auto field : row) {
      std::cout << " " << field.first << " => " << field.second << std::endl;
    }
    std::cout << "}" << std::endl;
  }  
}

void PrintResults(const std::vector<mprmpr::db::Result*>& results) {
  std::cout << "--------BEGIN results" << std::endl;
  for (auto r : results) {
    PrintResult(r);
  }
  std::cout << "--------END results" << std::endl;
}

int main() {
  mprmpr::db::ConnectionBuilder builder;
  std::unique_ptr<mprmpr::db::Connection> conn;

  mprmpr::Status s;

  s = builder.set_hostname(kHostname)
             .set_username(kUsername)
             .set_password(kPassword)
             .set_database(kDatabase)
             .Build(&conn);
  DCHECK(s.ok()) << " Connect: " << builder.hostname() << " Error: " << s.ToString();

  s = conn->Query("DROP TABLE IF EXISTS `account`");
  DCHECK(s.ok()) << "Query: " << s.ToString();

  s = conn->Query("CREATE TABLE account (id CHAR(4) NOT NULL, "
                  "name VARCHAR(30) NOT NULL, "
                  "registered_at DATETIME, "
                  "PRIMARY KEY (id))");
  DCHECK(s.ok()) << "Query: " << s.ToString();
  // Test Reconnt
  s = conn->Query("SET wait_timeout=5");
  DCHECK(s.ok()) << "Query: " << s.ToString();

  mprmpr::db::Statement statement(conn.get(), "INSERT INTO account (id, name, registered_at) VALUES (?, ?, NOW())");
  s = statement.Execute("001", "wqx");
  DCHECK(s.ok()) << "Query: " << s.ToString();
  s = statement.Execute("002", "lwz");
  DCHECK(s.ok()) << "Query: " << s.ToString();

  //sleep(10);

  s = conn->Query("SELECT * FROM account");  
  DCHECK(s.ok()) << "Query: " << s.ToString();
  PrintResults(conn->GetResults());

  //sleep(10);

  mprmpr::db::Statement select_statement(conn.get(),
		     "SELECT * FROM account");
  s = select_statement.Execute();
  DCHECK(s.ok()) << "Query: " << s.ToString();
  PrintResult(select_statement.GetResult());

  return 0;
}

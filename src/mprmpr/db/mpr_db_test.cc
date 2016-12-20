#include "mprmpr/db/db.h"

const std::string kHostname("localhost");
const std::string kUsername("root");
const std::string kPassword("111111");
const std::string kDatabase("test_db");

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
  auto s1 = conn->GetResults();

  s = conn->Query("CREATE TABLE account (id CHAR(4) NOT NULL, "
                  "name VARCHAR(30) NOT NULL, "
                  "PRIMARY KEY (id))");
  DCHECK(s.ok()) << "Query: " << s.ToString();
  auto s2 = conn->GetResults();

  mprmpr::db::Statement statement(conn.get(), "INSERT INTO account (id, name) VALUES (?, ?)");
  s = statement.Execute("001", "wqx");
  DCHECK(s.ok()) << "Query: " << s.ToString();
  s = statement.Execute("002", "lwz");
  DCHECK(s.ok()) << "Query: " << s.ToString();

  mprmpr::db::Statement select_statement(conn.get(),
		     "SELECT * FROM account");
  s = select_statement.Execute();
  DCHECK(s.ok()) << "Query: " << s.ToString();
  for (auto row : *select_statement.GetResult()) {
    std::cout << "{" << std::endl;
    for (auto field : row) {
      std::cout << " " << field.first << " => " << field.second << std::endl;
    }
    std::cout << "}" << std::endl;
  }  
  return 0;
}

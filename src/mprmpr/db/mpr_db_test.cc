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

  std::vector<mprmpr::db::Result> results;
  s = conn->Query(results, "DROP TABLE IF EXISTS `account`");
  DCHECK(s.ok()) << "Query: " << s.ToString();
  s = conn->Query(results, "CREATE TABLE account (id CHAR(4) NOT NULL, "
                           "name VARCHAR(30) NOT NULL, "
                           "PRIMARY KEY (id))");
  DCHECK(s.ok()) << "Query: " << s.ToString();

  std::unique_ptr<mprmpr::db::Result> result;
  mprmpr::db::Statement statement(conn.get(), "INSERT INTO account (id, name) VALUES (?, ?)");
  s = statement.Execute(&result, "001", "wqx");
  DCHECK(s.ok()) << "Query: " << s.ToString();

  return 0;
}

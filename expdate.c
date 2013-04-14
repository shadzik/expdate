/* 
 * Print out user accounts expiration date
 * using MySQL
 *
 * Author: Bartosz 'shadzik' Świątek (shadzik@atwa.us)
 *
 * gcc -I/usr/include/mysql -o expdate expdate.c -lmysqlclient -lconfig
 * strip expdate
 * cp expdate /usr/bin/expdate
 * the config file located at /etc/expdate.conf schould have chmod 600 rights
 */

#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <libconfig.h>

int main() {
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	register struct passwd *pw;
	register uid_t uid, ruid;
	uid = geteuid();
	ruid = getuid();
	pw = getpwuid(ruid);

	const char *server, *login, *password, *database;

	// read conf
	config_t cfg;
	config_setting_t *setting;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "/etc/expdate.conf"))
	{
	fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
	config_destroy(&cfg);
	return(EXIT_FAILURE);
	}

	/* Get settings. */
	if(!config_lookup_string(&cfg, "host", &server))
		fprintf(stderr, "No 'host' setting in configuration file.\n");
	if(!config_lookup_string(&cfg, "user", &login))
		fprintf(stderr, "No 'user' setting in configuration file.\n");
	if(!config_lookup_string(&cfg, "password", &password))
		fprintf(stderr, "No 'password' setting in configuration file.\n");
	if(!config_lookup_string(&cfg, "database", &database))
		fprintf(stderr, "No 'database' setting in configuration file.\n");

	int status;
	status = seteuid(ruid);
	if (status < 0) {
		fprintf (stderr, "Couldn't set uid.\n");
		exit (status);
	}
	char *user = (pw->pw_name);
	char *q = "select wygasa from users where login='";
	char *query;

	query = (char *)malloc((strlen(q) + strlen(user) + 2) * sizeof(char));
	strcpy(query, q);
	strcat(query, user);
	strcat(query, "'");

	//printf("User: %s\n", user);
	//printf("Query: %s\n", query);

	conn = mysql_init(NULL);

	// Connect to database
	//printf("Connecting to database\n");
	if (!mysql_real_connect(conn, server, login, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	// send SQL query
	//printf("Sending query\n");
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_use_result(conn);

	// output date
	//printf("Output date\n");
	while((row = mysql_fetch_row(res)) != NULL)
		printf("%s\n", row[0]);

	//close connection
	//printf("Close connection\n");
	mysql_free_result(res);
	mysql_close(conn);
}

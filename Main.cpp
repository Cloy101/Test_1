// Main.cpp

#ifdef NDEBUG
const bool test = false;
#else
const bool test = true;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <exception>
#include "libpq-fe.h"
#include <nlohmann/json.hpp>


using json = nlohmann::json;

void exit_nicely(PGconn* conn)
{
    PQfinish(conn);
    exit(1);
}

int main(int argc, char** argv)
{
    // 1.1. Read the file .json with library nlomahmann_json
    json data;
    
    try
    {
        std::ifstream config("config.json");
        config >> data;
        config.close();
    }

    catch (const std::exception& e)
    {
        std::cerr << "ERROR::The file config.json is unavailable" << e.what() << std::endl;
    }

    std::string pqDb = data["db"];
    
    std::string pqHost;
    if (test)
    {
        pqHost = "localhost";
    }   else 
        {
            std::string pqHost = data["connection"]["host"];
        }

    int jPort = data["connection"]["port"];
    std::string pqDatabase = data["connection"]["database"];
    std::string pqUser = data["connection"]["user"];
    std::string pqPass = data["connection"]["pass"];

    // 2.1 Create the connection to the PostgreSQL data base
   const std::string connInfo = "host=" + pqHost +
                            " port=" + std::to_string(jPort) +
                            " dbname=" + pqDatabase +
                            " user=" + pqUser +
                            " password=" + pqPass;

    PGconn* conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "ERROR::Connection to Database failed:  %s", PQerrorMessage(conn));
        exit_nicely(conn);
    }

    // 3.1. Make the variable to send executable command to the PostgreSQL Data base
    PGresult* res;
    const std::string path = "output.json";
    std::ofstream out(path);

    if (out.is_open())
    {
       // out << "[";

        res = PQexec(conn, "SELECT tablename FROM pg_tables WHERE schemaname = 'public';");
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "ERROR::Failed to fetch table names: %s", PQerrorMessage(conn));
            PQclear(res);
            exit_nicely(conn);
        }

        int numTables = PQntuples(res);
            
        for (int i = 0; i < numTables; ++i)
        {
            std::string tableName = PQgetvalue(res, i, 0);
            std::string copyCommand = "SELECT json_agg(t) FROM " + tableName + " t";

            res = PQexec(conn, copyCommand.c_str());
            if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0)
            {
                out << PQgetvalue(res, 0, 0);
                
                if (i < numTables - 1)
                {
                    out << std::endl << std::endl;
                    res = PQexec(conn, "SELECT tablename FROM pg_tables WHERE schemaname = 'public';");
                    if (PQresultStatus(res) != PGRES_TUPLES_OK)
                    {
                        fprintf(stderr, "ERROR::Failed to fetch table names: %s", PQerrorMessage(conn));
                        PQclear(res);
                        exit_nicely(conn);
                    }
                }

                
            }   else
                {
                    fprintf(stderr, "ERROR:: Failed to copy table %s to JSON: %s", tableName.c_str(), PQerrorMessage(conn));
                    PQclear(res);
                    exit_nicely(conn);
                }
        }
  
       // out << "]";
        out.close();
        PQclear(res);

    }   else 
        {
            fprintf(stderr, "ERROR:: Can't open the output.json");
        }
          
    PQfinish(conn);
}
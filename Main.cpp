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
#include <string>

void exit_nicely(PGconn* conn)
{
    PQfinish(conn);
    exit(1);
}

std::string readFile(const std::string& fileName)
{
    return std::string((std::istreambuf_iterator<char>(std::ifstream(fileName))), std::istreambuf_iterator<char>());
}

size_t findWord(std::string text, std::string word)
{
   return text.find(word) + word.length() + 3;
}

std::string copyWord(std::string text, std::string word)
{
    size_t start = findWord(text, word);
    if (text.at(start) == '\"')
    {
        start += 1;
    }

    std::string temp;

    while (text.at(start) != ',' && text.at(start) != '\"')
    {
        temp += text.at(start);
        ++start;
    }
    return temp;
}

int main(int argc, char** argv)
{
    // 1.1. Read the file .json with library nlomahmann_json
    std::string fileName = "config.json";
    std::string dataString;
    
    try
    {
        dataString = readFile(fileName);
    }

    catch (const std::exception& e)
    {
        std::cerr << "ERROR::The file config.json is unavailable" << e.what() << std::endl;
    }

    std::string pqHost = copyWord(dataString,"host");

    if (test)
    {
        pqHost = "localhost";
    }
    
    //2.1 Make the connection
    std::string pqDb = copyWord(dataString, "db");
    std::string pqPort = copyWord(dataString, "port");
    std::string pqDatabase = copyWord(dataString, "database");
    std::string pqUser = copyWord(dataString, "user");
    std::string pqPass = copyWord(dataString, "pass");

    const std::string connInfo = "host=" + pqHost +
       " port=" + pqPort +
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
        std::string temp = "{\n\"db\": \"" + pqDb + "\",\n";
        out << temp;

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
            std::string copyCommand = "SELECT json_agg(t) FROM " + tableName + " t;";

            res = PQexec(conn, copyCommand.c_str());
            if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0)
            {
                std::string temp2 = "\"" + tableName + "\": \n";
                out << temp2 << PQgetvalue(res, 0, 0);
                
                if (i < numTables - 1)
                {
                    out << ',' << '\n';

                    res = PQexec(conn, "SELECT tablename FROM pg_tables WHERE schemaname = 'public';");
                    if (PQresultStatus(res) != PGRES_TUPLES_OK)
                    {
                        fprintf(stderr, "ERROR::Failed to fetch table names: %s", PQerrorMessage(conn));
                        PQclear(res);
                        exit_nicely(conn);
                    }

                }   else
                    {
                        out << '\n' << '}';
                    }
            }   else
                {
                    fprintf(stderr, "ERROR:: Failed to copy table %s to JSON: %s", tableName.c_str(), PQerrorMessage(conn));
                    PQclear(res);
                    exit_nicely(conn);
                }
        }

        out.close();
        PQclear(res);

    }   else 
        {
            fprintf(stderr, "ERROR:: Can't open the output.json");
        }
          
    PQfinish(conn);
}
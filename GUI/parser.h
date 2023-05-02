
#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>
#include <unordered_map>

struct parserResult{
    std::string table;
    std::string selectedAttribute;
    std::string indexName;
    std::string indexValue;
    std::vector<std::string> selectedAttributes;
    std::unordered_map<std::string, std::string> atributos;
    std::string queryType;
    std::string range1;
    std::string range2;
    std::vector<std::string> cadena;
    std::vector<std::string> tables = {"movies"};
    std::vector<std::string> attributes = {"dataId", "contentType", "title", "length", "releaseYear", "endYear", "votes", "rating", "gross", "certificate", "description"};
    std::vector<std::string> indexes = {"ISAM", "Hash", "AVL"};

    void select_inst(){
        std::cout << "Table: " << table << std::endl;
        if(selectedAttributes.empty()) std::cout << "\nShown attributes: all\n";
        else{
            std::cout << "\nShown attributes:\n";
            for(const auto& atributo: selectedAttributes)
                std::cout << atributo << "\t";
            std::cout << "\n\nSearch by column: " << selectedAttribute << std::endl;

            std::cout << "\nWhere match: " << atributos[selectedAttribute] << std::endl;
        }
    }

    void insert_inst(){
        std::cout << "Table: " << table << std::endl;
        std::cout << "\nInsert a record which values are:\n";
        for(const auto& atributo: attributes){
            std::cout << atributos[atributo] << "\t";
        }
    }

    void delete_inst(){
        std::cout << "Table: " << table << std::endl;

        std::cout << "\nDelete registers by column: " << selectedAttribute << std::endl;

        std::cout << "\nWhere match: " << atributos[selectedAttribute] << std::endl;
    }

    void create_index() const{
        std::cout << "Indexed column: " << selectedAttribute << std::endl;
        std::cout << "\nIndex name: " << indexName << std::endl;
        std::cout << "\nIndex type: " << indexValue << std::endl;
    }

    parserResult() {
        for(const auto& atributo: attributes){
            atributos[atributo] = "";
        }
    }

    void killSelf(){
        table = "";
        selectedAttribute = "";
        indexName = "";
        indexValue = "";
        selectedAttributes.clear();
        atributos.clear();
    }

    virtual ~parserResult() {
        killSelf();
    }
};



struct parserSQL {

    std::string token;
    short pos = 0;
    parserResult parsero;
    std::string s;
    std::vector<std::string> cadena;
    std::vector<std::string> tables = {"movies"};
    std::vector<std::string> attributes = {"dataId", "contentType", "title", "length", "releaseYear", "endYear", "votes", "rating", "gross", "certificate", "description"};
    std::vector<std::string> indexes = {"ISAM", "Hash", "AVL"};

    void getToken() {
        token = cadena[pos];
        pos += 1;
    }

    void error() {
        pos = 0;
        parsero.killSelf();
        throw std::runtime_error("Cadena invalida.");
    }

    void match(const std::string &expectedToken) {
        if (token == expectedToken) getToken();
        else error();
    }

    void matchAttributes(const std::string &expectedToken) {
        std::vector<std::string>::iterator iter;
        iter = find(attributes.begin(), attributes.end(), expectedToken);
        parsero.selectedAttribute = token;
        if (iter != attributes.end()) std::cout << "";
        else error();
    }

    void matchTables(const std::string &expectedToken) {
        std::vector<std::string>::iterator iter;
        iter = find(tables.begin(), tables.end(), expectedToken);
        parsero.table = token;
        if (iter != tables.end()) std::cout << "";
        else error();
    }

    void matchIndex(const std::string &expectedToken) {
        std::vector<std::string>::iterator iter;
        iter = find(indexes.begin(), indexes.end(), expectedToken);
        parsero.indexValue = token;
        if (iter != indexes.end()) std::cout << "";
        else error();
    }

    void value() {

        if (regex_match(token, std::regex("(-?[0-9.a-zA-Z]*)")) ||
            regex_match(token, std::regex(R"((["(][()\s+a-z,.0-9A-Z:;\"\']*[")]))"))) {
            getToken();
        } else {
            error();
        }
    }

    void table() {
        matchTables(token);
        getToken();
    }

    void attribute() {
        matchAttributes(token);
        getToken();
    }

    void index() {
        matchIndex(token);
        getToken();
    }

    void list() {

        std::vector<std::string> v;
        std::stringstream ss(token);
        while (ss.good()) {
            std::string substr;
            getline(ss, substr, ',');
            v.push_back(substr);
        }

        for (const auto &word: v) {
            matchAttributes(word);
        }
        parsero.selectedAttributes.assign(v.begin(), v.end());
        getToken();
    }

    void insertValues() {
        std::vector<std::string> v;
        token.erase(token.begin());
        token.erase(token.end() - 1);
        std::stringstream ss(token);
        while (ss.good()) {
            std::string substr;
            getline(ss, substr, ',');
            v.push_back(substr);
        }
        int cont = 0;
        for (const auto &atributo: attributes) {
            parsero.atributos[atributo] = v[cont];
            cont++;
        }

        token = "(" + token + ")";
    }

    void select() {
        if (token == "*") {
            match("*");
        } else
            list();

        match("FROM");

        table();

        if (token == "WHERE") {
            match("WHERE");

            attribute();

            if(token == "="){
                match("=");

                parsero.atributos[parsero.selectedAttribute] = token;
                value();
            }
            else if(token == "BETWEEN"){
                match("BETWEEN");

                parsero.range1 = token;
                value();

                match("AND");

                parsero.range2 = token;
                value();
            }
            else error();
        }
    }

    void insert() {
        match("INTO");

        table();

        match("VALUES");

        insertValues();

        value();
    }

    void deletex() {
        match("FROM");

        table();

        match("WHERE");

        attribute();

        match("=");

        parsero.atributos[parsero.selectedAttribute] = token;
        value();
    }

    void create() {
        match("INDEX");

        parsero.indexName = token;
        value();

        match("ON");

        attribute();

        match("USING");

        index();
    }

    void action() {
        if (token == "SELECT") {
            match("SELECT");
            select();
            parsero.select_inst();
            parsero.queryType = "SELECT";
        } else if (token == "INSERT") {
            match("INSERT");
            insert();
            parsero.insert_inst();
            parsero.queryType = "INSERT";
        } else if (token == "DELETE") {
            match("DELETE");
            deletex();
            parsero.delete_inst();
            parsero.queryType = "DELETE";
        } else if (token == "CREATE") {
            match("CREATE");
            create();
            parsero.create_index();
            parsero.queryType = "CREATE";
        } else {
            error();
        }
    }


    parserResult query(std::string s){
        //INSERT only works if (abc,"a b c",123)
        //SELECT only works if contentType = "something"
        //INSERT only works if (123,2,3)
        //s = R"(CREATE INDEX indexito ON movies USING ISAM)";

        std::stringstream ss(s);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> cadena2(begin, end);
        cadena2.emplace_back("$");

        int cont = 0;
        while (cont < cadena2.size() - 1) {
            if ((cadena2[cont][0] == '\"' or cadena2[cont][0] == '(') &&
                cadena2[cont][cadena2[cont].size() - 1] != '\"') {
                if (cadena2[cont][1] == ')') error();
                if (cadena2[cont + 1] == "$") std::cout << "";
                else {
                    do {
                        cadena2[cont] = cadena2[cont] + " " + cadena2[cont + 1];
                        int cont2 = 1;

                        while (cont + cont2 < cadena2.size() - 1) {
                            cadena2[cont + cont2] = cadena2[cont + cont2 + 1];
                            cont2++;
                        }

                        cadena2.resize(cadena2.size() - 1);

                        if (cadena2[cont][cadena2[cont].size() - 1] == ')' or
                            cadena2[cont][cadena2[cont].size() - 1] == '\"')
                            break;
                    } while (true);
                }
            }
            cont++;
        }


        cadena.assign(cadena2.begin(), cadena2.end());

        getToken();
        action();

        if (token == "$") std::cout << "\nCadena valida\n";
        else error();
        pos = 0;
        return parsero;
    }
};


#endif // PARSER_H

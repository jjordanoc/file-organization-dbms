#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <regex>
#include <unordered_map>
using namespace std;

vector<string> cadena;
vector<string> tables = {"movies"};
vector<string> attributes = {"dataId", "contentType", "title", "length", "releaseYear", "endYear", "votes", "rating", "gross", "certificate", "description"};
vector<string> indexes = {"ISAM", "Hash", "AVL"};
struct parserator{
    string table;
    string selectedAttribute;
    string indexName;
    string indexValue;
    vector<string> selectedAttributes;
    unordered_map<string, string> atributos;
    string queryType;

    void select_inst(){
        cout << "Table: " << table << endl;
        if(selectedAttributes.empty()) cout << "\nShown attributes: all\n";
        else{
            cout << "\nShown attributes:\n";
            for(const auto& atributo: selectedAttributes)
                cout << atributo << "\t";
            cout << "\n\nSearch by column: " << selectedAttribute << endl;

            cout << "\nWhere match: " << atributos[selectedAttribute] << endl;
        }
    }

    void insert_inst(){
        cout << "Table: " << table << endl;
        cout << "\nInsert a record which values are:\n";
        for(const auto& atributo: attributes){
            cout << atributos[atributo] << "\t";
        }
    }

    void delete_inst(){
        cout << "Table: " << table << endl;

        cout << "\nDelete registers by column: " << selectedAttribute << endl;

        cout << "\nWhere match: " << atributos[selectedAttribute] << endl;
    }

    void create_index(){
        cout << "Indexed column: " << selectedAttribute << endl;
        cout << "\nIndex name: " << indexName << endl;
        cout << "\nIndex type: " << indexValue << endl;
    }

    parserator() {
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

    virtual ~parserator() {
        killSelf();
    }
};

parserator parsero;

string token;
short pos = 0;

void getToken(){
    token = cadena[pos];
    pos += 1;
}

void error(){
    cout << "\nError: Cadena invalida\n";
    exit(1);
}

void match(const string& expectedToken){
    if(token == expectedToken)  getToken();
    else    error();
}

void matchAttributes(const string& expectedToken){
    vector<string>::iterator iter;
    iter = find(attributes.begin(),attributes.end(),expectedToken);
    parsero.selectedAttribute = token;
    if(iter != attributes.end()) cout << "";
    else    error();
}

void matchTables(const string& expectedToken){
    vector<string>::iterator iter;
    iter = find(tables.begin(),tables.end(),expectedToken);
    parsero.table = token;
    if(iter != tables.end()) cout << "";
    else    error();
}

void matchIndex(const string& expectedToken){
    vector<string>::iterator iter;
    iter = find(indexes.begin(),indexes.end(),expectedToken);
    parsero.indexValue = token;
    if(iter != indexes.end()) cout << "";
    else    error();
}

void value(){

    if (regex_match(token,regex("([0-9.a-zA-Z]*)")) || regex_match(token,regex(R"((["(][()\s+a-z,.0-9A-Z:;\"\']*[")]))"))){
        getToken();
    }
    else{
        error();
    }
}

void table(){
    matchTables(token);
    getToken();
}

void attribute(){
    matchAttributes(token);
    getToken();
}

void index(){
    matchIndex(token);
    getToken();
}
void list(){

    vector<string> v;
    stringstream ss(token);
    while (ss.good()) {
        string substr;
        getline(ss, substr, ',');
        v.push_back(substr);
    }

    for(const auto& word: v){
        matchAttributes(word);
    }
    parsero.selectedAttributes.assign(v.begin(),v.end());
    getToken();
}

void insertValues(){
    vector<string> v;
    token.erase(token.begin());
    token.erase(token.end()-1);
    stringstream ss(token);
    while (ss.good()) {
        string substr;
        getline(ss, substr, ',');
        v.push_back(substr);
    }
    int cont = 0;
    for(const auto& atributo: attributes){
        parsero.atributos[atributo] = v[cont];
        cont++;
    }

    token = "(" + token + ")";
}

void select(){
    if (token == "*"){
        match("*");
    }
    else
        list();

    match("FROM");

    table();

    if(token == "WHERE") {
        match("WHERE");

        attribute();

        match("=");

        parsero.atributos[parsero.selectedAttribute] = token;
        value();
    }
}

void insert(){
    match("INTO");

    table();

    match("VALUES");

    insertValues();

    value();
}

void deletex(){
    match("FROM");

    table();

    match("WHERE");

    attribute();

    match("=");

    parsero.atributos[parsero.selectedAttribute] = token;
    value();
}

void create(){
    match("INDEX");

    parsero.indexName = token;
    value();

    match("ON");

    attribute();

    match("USING");

    index();
}
void action(){
    if(token == "SELECT"){
        match("SELECT");
        select();
        parsero.select_inst(); parsero.queryType = "SELECT";
    }
    else if(token == "INSERT"){
        match("INSERT");
        insert();
        parsero.insert_inst(); parsero.queryType = "INSERT";
    }
    else if(token == "DELETE"){
        match("DELETE");
        deletex();
        parsero.delete_inst(); parsero.queryType = "DELETE";
    }
    else if(token == "CREATE"){
        match("CREATE");
        create();
        parsero.create_index(); parsero.queryType = "CREATE";
    }
    else {
        error();
    }
}



int main() {
    //{"dataId", "contentType", "title", "length", "releaseYear", "endYear", "votes", "rating", "gross", "certificate", "description"}
    string s;
    //INSERT only works if (abc,"a b c",123)
    //SELECT only works if contentType = "something"
    //INSERT only works if (123,2,3)
    //s = R"(CREATE INDEX indexito ON movies USING ISAM)";

    while(s != "exit") {
        cout << string(20,'-');
        cout << "\nQuery: \n";
        pos = 0;
        getline(cin, s);
        cout << string(20,'-') << endl;
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
                if (cadena2[cont + 1] == "$") cout << "";
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

        if (token == "$") cout << "\nCadena valida\n";
        else error();



    }
    return 0;
}


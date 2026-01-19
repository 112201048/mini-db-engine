//Client for the DB engine
#include <iostream>
#include "storage/TableFile.h"
#include <vector>
#include <string>

using namespace std;

int main() {
    try {
        TableFile tableFile("datatest.db");

        // Insert some rows
        tableFile.insertRow({"Alice", "30", "Engineer"});
        tableFile.insertRow({"Bob", "25", "Designer"});
        tableFile.insertRow({"Charlie", "35", "Manager"});

        // Read all rows
        vector<vector<string>> rows = tableFile.scanAll();
        for (const auto& row : rows) {
            for (const auto& col : row) {
                cout << col << " ";
            }
            cout << endl;
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
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
        RID r1 = tableFile.insertRow({"Alice", "30", "Engineer"});
        RID r2 = tableFile.insertRow({"Bob", "25", "Designer"});
        RID r3 = tableFile.insertRow({"Charlie", "35", "Manager"});
        RID r4 = tableFile.insertRow({"Diana", "28", "Analyst"});

        vector<string> rowexample = tableFile.getRow(r2); // Example of retrieving a single row
        cout << "Retrieved Row: ";
        for (const auto& col : rowexample) {
            cout << col << " ";
        }
        cout << endl;

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
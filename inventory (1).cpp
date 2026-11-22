#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

// ============================================================
// CSC 101 FINAL PROJECT
// Inventory Sorting & Warehouse Tracking System
// Option 2: 8 x 8 warehouse grid, inventory management,
// automatic consolidation, category reports, and recursive
// shortest-path search from entrance [0][0].
// ============================================================

const int ROWS = 8;
const int COLS = 8;
const int NUM_CATEGORIES = 4;
const int INF = numeric_limits<int>::max() / 4;
const int LOW_STOCK_THRESHOLD = 10;

enum Category {
    ELECTRONICS = 0,
    GROCERY,
    TOOLS,
    APPAREL
};

const string CATEGORY_NAMES[NUM_CATEGORIES] = {
    "Electronics", "Grocery", "Tools", "Apparel"
};

enum AddResult {
    ADD_NEW,
    ADD_RESTOCK,
    ADD_FULL,
    ADD_INVALID,
    ADD_OVERFLOW
};

enum UpdateResult {
    UPD_OK,
    UPD_REMOVED,
    UPD_NOT_FOUND,
    UPD_INSUFFICIENT,
    UPD_OVERFLOW
};

struct Cell {
    bool blocked = false;
    bool entrance = false;
    int id = 0;
    int qty = 0;
    int category = -1;
    double price = 0.0;
    string name;
};

using Grid = Cell[ROWS][COLS];

struct Position {
    int row;
    int col;
};

using DistanceMap = int[ROWS][COLS];

const int DR[4] = {-1, 1, 0, 0};
const int DC[4] = {0, 0, -1, 1};

// -------------------- Basic cell helpers --------------------

bool isStorage(const Cell& cell) {
    return !cell.blocked && !cell.entrance;
}

bool isEmpty(const Cell& cell) {
    return cell.id == 0;
}

bool validCategory(int category) {
    return category >= 0 && category < NUM_CATEGORIES;
}

void clearItem(Cell& cell) {
    cell.id = 0;
    cell.qty = 0;
    cell.category = -1;
    cell.price = 0.0;
    cell.name.clear();
}

// -------------------- Warehouse initialization --------------------

void initWarehouse(Grid& grid) {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            grid[r][c] = Cell{};
        }
    }

    // Fixed warehouse floor plan. '#' cells are blocked aisles.
    // The entrance is always at [0][0].
    const string layout[ROWS] = {
        "E.......",
        "...#....",
        "...#....",
        "........",
        ".##.....",
        "........",
        ".....#..",
        "........"
    };

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (layout[r][c] == '#') {
                grid[r][c].blocked = true;
            } else if (layout[r][c] == 'E') {
                grid[r][c].entrance = true;
            }
        }
    }
}

bool findItem(const Grid& grid, int id, int& foundRow, int& foundCol) {
    if (id <= 0) return false;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (grid[r][c].id == id) {
                foundRow = r;
                foundCol = c;
                return true;
            }
        }
    }
    return false;
}

// -------------------- Sorting and consolidation --------------------

void consolidateGrid(Grid& grid) {
    vector<Cell> items;

    // Collect all stored items without changing the floor plan.
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (isStorage(grid[r][c]) && !isEmpty(grid[r][c])) {
                items.push_back(grid[r][c]);
                clearItem(grid[r][c]);
            }
        }
    }

    // Refill available storage cells in row-major order.
    size_t index = 0;
    for (int r = 0; r < ROWS && index < items.size(); ++r) {
        for (int c = 0; c < COLS && index < items.size(); ++c) {
            if (isStorage(grid[r][c]) && isEmpty(grid[r][c])) {
                grid[r][c] = items[index++];
            }
        }
    }
}

void sortGrid(Grid& grid) {
    vector<Cell> items;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (isStorage(grid[r][c]) && !isEmpty(grid[r][c])) {
                items.push_back(grid[r][c]);
                clearItem(grid[r][c]);
            }
        }
    }

    sort(items.begin(), items.end(),
         [](const Cell& a, const Cell& b) {
             if (a.category != b.category) return a.category < b.category;
             if (a.name != b.name) return a.name < b.name;
             return a.id < b.id;
         });

    size_t index = 0;
    for (int r = 0; r < ROWS && index < items.size(); ++r) {
        for (int c = 0; c < COLS && index < items.size(); ++c) {
            if (isStorage(grid[r][c]) && isEmpty(grid[r][c])) {
                grid[r][c] = items[index++];
            }
        }
    }
}

void reorganize(Grid& grid) {
    consolidateGrid(grid);
    sortGrid(grid);
}

// -------------------- Inventory operations --------------------

AddResult addItem(Grid& grid, int id, const string& name,
                  int qty, double price, int category) {
    if (id <= 0 || name.empty() || qty <= 0 ||
        !isfinite(price) || price < 0.0 ||
        !validCategory(category)) {
        return ADD_INVALID;
    }

    int r = 0;
    int c = 0;

    // If the ID already exists, add the new quantity to its stock.
    if (findItem(grid, id, r, c)) {
        long long newQty = static_cast<long long>(grid[r][c].qty) + qty;
        if (newQty > numeric_limits<int>::max()) return ADD_OVERFLOW;
        grid[r][c].qty = static_cast<int>(newQty);
        return ADD_RESTOCK;
    }

    // Find the first available storage cell.
    for (r = 0; r < ROWS; ++r) {
        for (c = 0; c < COLS; ++c) {
            if (isStorage(grid[r][c]) && isEmpty(grid[r][c])) {
                grid[r][c].id = id;
                grid[r][c].name = name;
                grid[r][c].qty = qty;
                grid[r][c].price = price;
                grid[r][c].category = category;
                reorganize(grid);
                return ADD_NEW;
            }
        }
    }

    return ADD_FULL;
}

bool removeItem(Grid& grid, int id) {
    int r = 0;
    int c = 0;

    if (!findItem(grid, id, r, c)) return false;

    clearItem(grid[r][c]);
    consolidateGrid(grid);
    return true;
}

UpdateResult updateQuantity(Grid& grid, int id, int delta) {
    int r = 0;
    int c = 0;

    if (!findItem(grid, id, r, c)) return UPD_NOT_FOUND;

    long long newQty = static_cast<long long>(grid[r][c].qty) + delta;

    if (newQty < 0) return UPD_INSUFFICIENT;
    if (newQty > numeric_limits<int>::max()) return UPD_OVERFLOW;

    if (newQty == 0) {
        removeItem(grid, id);
        return UPD_REMOVED;
    }

    grid[r][c].qty = static_cast<int>(newQty);
    return UPD_OK;
}

// -------------------- Reports --------------------

void categoryValuations(const Grid& grid, double totals[NUM_CATEGORIES],
                        int counts[NUM_CATEGORIES]) {
    for (int i = 0; i < NUM_CATEGORIES; ++i) {
        totals[i] = 0.0;
        counts[i] = 0;
    }

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            const Cell& cell = grid[r][c];

            if (!isEmpty(cell) && validCategory(cell.category)) {
                totals[cell.category] +=
                    static_cast<double>(cell.qty) * cell.price;
                counts[cell.category]++;
            }
        }
    }
}

int lowStockAlerts(const Grid& grid, int threshold,
                   vector<string>& alerts) {
    alerts.clear();
    if (threshold < 0) return 0;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            const Cell& cell = grid[r][c];

            if (!isEmpty(cell) && cell.qty <= threshold) {
                ostringstream message;
                message << cell.name << " (ID " << cell.id
                        << ") - stock: " << cell.qty
                        << " at [" << r << "][" << c << "]";
                alerts.push_back(message.str());
            }
        }
    }
    return static_cast<int>(alerts.size());
}

// -------------------- Recursive shortest-path search --------------------

// Recursive flood fill. Every move has a cost of one step.
// A cell is revisited only when a shorter distance is found.
void findShortestPath(const Grid& grid, int row, int col,
                      int currentDistance, DistanceMap& distance) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return;
    if (grid[row][col].blocked) return;
    if (currentDistance >= distance[row][col]) return;

    distance[row][col] = currentDistance;

    for (int direction = 0; direction < 4; ++direction) {
        int nextRow = row + DR[direction];
        int nextCol = col + DC[direction];

        if (nextRow >= 0 && nextRow < ROWS &&
            nextCol >= 0 && nextCol < COLS &&
            !grid[nextRow][nextCol].blocked) {
            findShortestPath(grid, nextRow, nextCol,
                             currentDistance + 1, distance);
        }
    }
}

bool getPathTo(const Grid& grid, int targetRow, int targetCol,
               vector<Position>& path) {
    path.clear();

    if (targetRow < 0 || targetRow >= ROWS ||
        targetCol < 0 || targetCol >= COLS) {
        return false;
    }

    if (grid[targetRow][targetCol].blocked) return false;

    DistanceMap distance;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            distance[r][c] = INF;
        }
    }

    findShortestPath(grid, 0, 0, 0, distance);

    if (distance[targetRow][targetCol] == INF) return false;

    // Trace backward from the target using a neighboring cell
    // whose distance is exactly one less.
    int row = targetRow;
    int col = targetCol;
    path.push_back({row, col});

    while (!(row == 0 && col == 0)) {
        bool moved = false;

        for (int direction = 0; direction < 4; ++direction) {
            int previousRow = row + DR[direction];
            int previousCol = col + DC[direction];

            if (previousRow >= 0 && previousRow < ROWS &&
                previousCol >= 0 && previousCol < COLS &&
                !grid[previousRow][previousCol].blocked &&
                distance[previousRow][previousCol] == distance[row][col] - 1) {
                row = previousRow;
                col = previousCol;
                path.push_back({row, col});
                moved = true;
                break;
            }
        }

        if (!moved) {
            path.clear();
            return false;
        }
    }

    reverse(path.begin(), path.end());
    return true;
}

// -------------------- Display functions --------------------

void displayGrid(const Grid& grid) {
    cout << "\nWAREHOUSE FLOOR GRID\n";
    cout << "E = Entrance | # = Blocked aisle | . = Empty | Number = Item ID\n\n";

    cout << "       ";
    for (int c = 0; c < COLS; ++c) {
        cout << setw(7) << c;
    }
    cout << '\n';

    for (int r = 0; r < ROWS; ++r) {
        cout << "Row " << r << "  ";
        for (int c = 0; c < COLS; ++c) {
            const Cell& cell = grid[r][c];

            string value;
            if (cell.blocked) value = "#";
            else if (cell.entrance) value = "E";
            else if (isEmpty(cell)) value = ".";
            else value = to_string(cell.id);

            cout << setw(7) << value;
        }
        cout << '\n';
    }
}

void listInventory(const Grid& grid) {
    cout << "\n";
    cout << left << setw(10) << "ID"
         << setw(23) << "Item Name"
         << right << setw(9) << "Qty"
         << setw(16) << "Category"
         << setw(13) << "Price"
         << setw(15) << "Location" << '\n';
    cout << string(86, '-') << '\n';

    bool found = false;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            const Cell& cell = grid[r][c];
            if (isEmpty(cell)) continue;

            found = true;
            cout << left << setw(10) << cell.id
                 << setw(23) << cell.name.substr(0, 21)
                 << right << setw(9) << cell.qty
                 << setw(16) << (validCategory(cell.category)
                                  ? CATEGORY_NAMES[cell.category] : "Unknown")
                 << setw(13) << fixed << setprecision(2) << cell.price
                 << setw(8) << "[" << r << "," << c << "]" << '\n';
        }
    }

    if (!found) cout << "No inventory items found.\n";
    cout << right;
}

void printDirections(const vector<Position>& path) {
    cout << "\nShortest route from entrance [0][0]:\n";
    for (size_t i = 0; i < path.size(); ++i) {
        cout << "[" << path[i].row << "][" << path[i].col << "]";
        if (i + 1 < path.size()) cout << " -> ";
    }
    cout << "\nTotal steps: " << (path.empty() ? 0 : path.size() - 1)
         << '\n';
}

// -------------------- Safe input functions --------------------

string readLine(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

int readInt(const string& prompt) {
    while (true) {
        string line = readLine(prompt);
        stringstream input(line);
        int value;
        char extra;

        if (input >> value && !(input >> extra)) return value;
        cout << "  Invalid input. Please enter a whole number.\n";
    }
}

double readDouble(const string& prompt) {
    while (true) {
        string line = readLine(prompt);
        stringstream input(line);
        double value;
        char extra;

        if (input >> value && !(input >> extra) && isfinite(value)) {
            return value;
        }
        cout << "  Invalid input. Please enter a valid number.\n";
    }
}

int readCategory() {
    cout << "\nCategories:\n";
    for (int i = 0; i < NUM_CATEGORIES; ++i) {
        cout << "  " << (i + 1) << ". " << CATEGORY_NAMES[i] << '\n';
    }

    while (true) {
        int choice = readInt("Choose category (1-4): ");
        if (choice >= 1 && choice <= NUM_CATEGORIES) return choice - 1;
        cout << "  Please choose a category from 1 to 4.\n";
    }
}

// -------------------- Menu actions --------------------

void menuAdd(Grid& grid) {
    cout << "\nADD NEW ITEM / RESTOCK ITEM\n";

    int id = readInt("Item ID (positive number): ");
    string name = readLine("Item name: ");
    int qty = readInt("Quantity to add: ");
    double price = readDouble("Unit price: ");
    int category = readCategory();

    AddResult result = addItem(grid, id, name, qty, price, category);

    switch (result) {
        case ADD_NEW:
            cout << "  New item added. Warehouse was reorganized.\n";
            break;
        case ADD_RESTOCK:
            cout << "  Existing item restocked successfully.\n";
            break;
        case ADD_FULL:
            cout << "  No available storage cell. Item was not added.\n";
            break;
        case ADD_INVALID:
            cout << "  Invalid information. Check ID, name, quantity, price, and category.\n";
            break;
        case ADD_OVERFLOW:
            cout << "  Quantity is too large. Stock was not changed.\n";
            break;
    }
}

void menuRemove(Grid& grid) {
    int id = readInt("\nEnter item ID to remove: ");

    if (removeItem(grid, id)) {
        cout << "  Item removed. Remaining items were consolidated.\n";
    } else {
        cout << "  Item ID not found.\n";
    }
}

void menuUpdate(Grid& grid) {
    int id = readInt("\nEnter item ID: ");
    int delta = readInt("Enter quantity change (+ to add, - to subtract): ");

    UpdateResult result = updateQuantity(grid, id, delta);

    switch (result) {
        case UPD_OK:
            cout << "  Quantity updated successfully.\n";
            break;
        case UPD_REMOVED:
            cout << "  Quantity reached zero. Item was removed and grid consolidated.\n";
            break;
        case UPD_NOT_FOUND:
            cout << "  Item ID not found.\n";
            break;
        case UPD_INSUFFICIENT:
            cout << "  Not enough stock. Quantity was not changed.\n";
            break;
        case UPD_OVERFLOW:
            cout << "  Quantity is too large. Stock was not changed.\n";
            break;
    }
}

void menuLookup(const Grid& grid) {
    int id = readInt("\nEnter item ID to search: ");
    int r = 0;
    int c = 0;

    if (!findItem(grid, id, r, c)) {
        cout << "  Item not found.\n";
        return;
    }

    const Cell& item = grid[r][c];
    cout << "\nItem found!\n";
    cout << "  ID:       " << item.id << '\n';
    cout << "  Name:     " << item.name << '\n';
    cout << "  Quantity: " << item.qty << '\n';
    cout << "  Category: " << CATEGORY_NAMES[item.category] << '\n';
    cout << "  Price:    " << fixed << setprecision(2) << item.price << '\n';
    cout << "  Location: [" << r << "][" << c << "]\n";
}

void menuPickOrder(const Grid& grid) {
    int id = readInt("\nEnter item ID to pick: ");
    int qty = readInt("Quantity to pick: ");

    if (qty <= 0) {
        cout << "  Quantity must be positive.\n";
        return;
    }

    int r = 0;
    int c = 0;

    if (!findItem(grid, id, r, c)) {
        cout << "  Item not found.\n";
        return;
    }

    if (grid[r][c].qty < qty) {
        cout << "  Insufficient stock. Available quantity: "
             << grid[r][c].qty << '\n';
        return;
    }

    vector<Position> path;
    if (!getPathTo(grid, r, c, path)) {
        cout << "  No valid route to this shelf.\n";
        return;
    }

    cout << "\nOrder can be picked.\n";
    cout << "Item: " << grid[r][c].name << '\n';
    cout << "Quantity: " << qty << '\n';
    printDirections(path);
}

void menuCategoryValuation(const Grid& grid) {
    double totals[NUM_CATEGORIES];
    int counts[NUM_CATEGORIES];

    categoryValuations(grid, totals, counts);

    cout << "\nCATEGORY INVENTORY VALUATION\n";
    cout << left << setw(18) << "Category"
         << right << setw(15) << "Item Types"
         << setw(20) << "Total Value" << '\n';
    cout << string(53, '-') << '\n';

    double grandTotal = 0.0;
    for (int i = 0; i < NUM_CATEGORIES; ++i) {
        cout << left << setw(18) << CATEGORY_NAMES[i]
             << right << setw(15) << counts[i]
             << setw(20) << fixed << setprecision(2) << totals[i] << '\n';
        grandTotal += totals[i];
    }

    cout << string(53, '-') << '\n';
    cout << left << setw(33) << "GRAND TOTAL"
         << right << setw(20) << fixed << setprecision(2)
         << grandTotal << '\n';
}

void menuLowStock(const Grid& grid) {
    cout << "\nLOW-STOCK ALERTS\n";
    cout << "Default low-stock threshold: "
         << LOW_STOCK_THRESHOLD << " units or fewer.\n";

    vector<string> alerts;
    int count = lowStockAlerts(grid, LOW_STOCK_THRESHOLD, alerts);

    if (count == 0) {
        cout << "No low-stock items found.\n";
        return;
    }

    cout << count << " low-stock alert(s):\n";
    for (const string& alert : alerts) {
        cout << "  - " << alert << '\n';
    }
}

void loadSampleData(Grid& grid) {
    addItem(grid, 1001, "Laptop", 8, 35000.00, ELECTRONICS);
    addItem(grid, 1002, "USB Cable", 25, 150.00, ELECTRONICS);
    addItem(grid, 1003, "Rice 5kg", 12, 280.00, GROCERY);
    addItem(grid, 1004, "Canned Goods", 7, 45.00, GROCERY);
    addItem(grid, 1005, "Hammer", 5, 320.00, TOOLS);
    addItem(grid, 1006, "Screwdriver", 14, 120.00, TOOLS);
    addItem(grid, 1007, "T-Shirt", 20, 250.00, APPAREL);
    addItem(grid, 1008, "Jacket", 4, 950.00, APPAREL);
    addItem(grid, 1009, "Keyboard", 9, 650.00, ELECTRONICS);
    addItem(grid, 1010, "Cooking Oil", 16, 95.00, GROCERY);
}

// -------------------- Main program --------------------

void displayMenu() {
    cout << "\n";
    cout << "====================================================\n";
    cout << "       INVENTORY SORTING & WAREHOUSE TRACKING\n";
    cout << "====================================================\n";
    cout << "  1. Display warehouse grid\n";
    cout << "  2. List all inventory items\n";
    cout << "  3. Add new item / restock item\n";
    cout << "  4. Remove item\n";
    cout << "  5. Update item quantity\n";
    cout << "  6. Search item by ID\n";
    cout << "  7. Pick order and show shortest route\n";
    cout << "  8. Show category inventory valuation\n";
    cout << "  9. Show low-stock alerts\n";
    cout << " 10. Re-sort and consolidate warehouse\n";
    cout << "  0. Exit\n";
    cout << "----------------------------------------------------\n";
}

int main() {
    Grid warehouse;
    initWarehouse(warehouse);
    loadSampleData(warehouse);

    cout << "Welcome to the Inventory Sorting & Warehouse Tracking System!\n";
    cout << "Sample inventory data has been loaded.\n";

    while (true) {
        displayMenu();
        int choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                displayGrid(warehouse);
                break;
            case 2:
                listInventory(warehouse);
                break;
            case 3:
                menuAdd(warehouse);
                break;
            case 4:
                menuRemove(warehouse);
                break;
            case 5:
                menuUpdate(warehouse);
                break;
            case 6:
                menuLookup(warehouse);
                break;
            case 7:
                menuPickOrder(warehouse);
                break;
            case 8:
                menuCategoryValuation(warehouse);
                break;
            case 9:
                menuLowStock(warehouse);
                break;
            case 10:
                reorganize(warehouse);
                cout << "  Warehouse successfully sorted and consolidated.\n";
                break;
            case 0:
                cout << "\nThank you for using the system. Goodbye!\n";
                return 0;
            default:
                cout << "  Invalid menu choice. Please select 0 to 10.\n";
        }
    }
}

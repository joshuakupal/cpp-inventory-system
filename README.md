# Inventory Sorting & Warehouse Tracking System

A console-based Inventory Management and Warehouse Tracking System developed in C++ as a CSC 101 Final Project.

The system combines inventory management, warehouse organization, stock monitoring, category-based valuation, and recursive shortest-path searching in an 8×8 warehouse grid.

## Project Overview

The Inventory Sorting & Warehouse Tracking System is designed to manage products stored in a simulated warehouse. It allows users to add, remove, search, update, and organize inventory while tracking product locations within a predefined warehouse layout.

It also includes automatic warehouse reorganization and a recursive pathfinding algorithm that determines the shortest route from the warehouse entrance to a selected item.

## Features

### 1. Inventory Management
- Add new inventory items.
- Restock existing products.
- Remove items from inventory.
- Update product quantities.
- Search for products using their unique ID.

### 2. Warehouse Grid System
- Uses an 8×8 warehouse layout.
- Displays product locations through a grid.
- Identifies blocked aisles and the warehouse entrance.
- Automatically assigns available storage cells.

### 3. Automatic Sorting and Consolidation
- Automatically reorganizes inventory when new products are added.
- Consolidates remaining products after item removal.
- Sorts products by category, item name, and ID.

### 4. Recursive Shortest-Path Search
- Uses recursive flood-fill traversal.
- Calculates the shortest accessible route from entrance `[0][0]`.
- Avoids blocked warehouse cells.
- Displays the route and total number of steps when picking an order.

### 5. Inventory Reports
- Calculates inventory valuation by category.
- Displays the total inventory value.
- Monitors low-stock products.
- Generates alerts for products with 10 units or fewer.

## Technologies Used

- **Programming Language:** C++
- **C++ Standard:** C++17
- **Data Structures:** Arrays, Structures, Vectors
- **Algorithms:** Sorting, Searching, Recursive Flood Fill
- **Libraries:** iostream, iomanip, string, vector, array, algorithm, sstream, limits, and cmath

## Warehouse Layout

The warehouse uses an 8×8 grid with the following symbols:

| Symbol | Description |
|---|---|
| E | Warehouse entrance |
| # | Blocked aisle |
| . | Empty storage location |
| Number | Inventory item ID |

The entrance is located at coordinate `[0][0]`.

## Inventory Categories

The system supports four inventory categories:

1. Electronics
2. Grocery
3. Tools
4. Apparel

## How to Run

### Requirements
- A C++ compiler (GCC recommended).
- A terminal or command prompt.
- Visual Studio Code (optional).

### Compilation

Open your terminal in the project directory and run:

```bash
g++ -std=c++17 -Wall -Wextra inventory.cpp -o inventory
```

### Execution

**Windows:**
```bash
.\inventory.exe
```

**Linux/macOS:**
```bash
./inventory
```

## Main Menu

The program provides the following operations:

| Option | Function |
|---|---|
| 1 | Display warehouse grid |
| 2 | List all inventory items |
| 3 | Add new item / Restock item |
| 4 | Remove item |
| 5 | Update item quantity |
| 6 | Search item by ID |
| 7 | Pick order and show shortest route |
| 8 | Show category inventory valuation |
| 9 | Show low-stock alerts |
| 10 | Re-sort and consolidate warehouse |
| 0 | Exit the program |

## Sample Inventory

The program automatically loads sample inventory data when it starts, including:

- Laptop
- USB Cable
- Rice 5kg
- Canned Goods
- Hammer
- Screwdriver
- T-Shirt
- Jacket
- Keyboard
- Cooking Oil

## Learning Objectives

This project demonstrates the practical application of fundamental C++ programming concepts, including:

- Functions and modular programming
- Structures and enumerations
- Two-dimensional arrays
- Vectors and sorting algorithms
- Conditional statements and loops
- Input validation and error handling
- Recursive algorithms
- Grid-based searching and pathfinding

## Project Information

**Project:** Inventory Sorting & Warehouse Tracking System  
**Course:** CSC 101  
**Project Type:** Final Programming Project  
**Language:** C++  
**Version:** 1.0

---

*Developed as an academic programming project to demonstrate inventory management, data organization, sorting algorithms, and recursive pathfinding using C++.*

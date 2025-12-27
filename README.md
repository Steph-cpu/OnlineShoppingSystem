# Shopping Management System

This is a C++ console application designed to manage product inventory, user accounts, and shopping transactions. The system focuses on data accuracy and a smooth workflow for both managers and customers.

## Key Features

* **Product Management:** Organize products efficiently using fast lookup methods.
* **User Levels:** Track user spending and automatically upgrade membership levels.
* **Safe Checkout:** A logic-controlled process that ensures inventory and user data are updated only when a purchase is finished.
* **Inventory Control:** Prevents products from being "lost" or incorrectly reduced during incomplete sessions.

## How to Use

### Prerequisites

* A C++ compiler (like GCC or Clang) that supports C++17 or later.

### Setup and Running

1. **Download the files** to your local computer.
2. **Open your terminal** and navigate to the project folder.
3. **Compile the program** using the following command:
```bash
g++ -std=c++17 Product.cpp ProductManager.cpp ShoppingCart.cpp User.cpp Menu.cpp Transaction.cpp -o ShoppingSystem
```


4. **Launch the app**:
```bash
./ShoppingSystem
```



---

## Challenges & Solutions

During the project, we faced several technical problems that helped us improve the system:

* **Duplicate Products:** At first, the system only checked Product IDs. This meant two different products could have the same name, which confused users. We solved this by adding a special "name-to-ID" map. Now, the system checks for name duplicates and allows users to search for items easily by name.
* **Data Accuracy:** We originally updated user profiles and stock levels as soon as items were added to the cart. However, if a user canceled the checkout, the data became incorrect. We fixed this by **delaying the updates**. Now, the system only changes stock and spending levels *after* the transaction is confirmed.

---

## Reflection and Future Plans

If we were to start over, we would focus more on decoupling the code. This means making sure each part of the program (like the database and the user interface) is more independent, making it easier to fix or update in the future.

### Next Steps

* **Create a GUI:** Move away from the text-based terminal to a visual window (Graphical User Interface).
* **Multi-user Support:** Use internet technology to let many people shop at the same time.
* **Code Refactoring:** Improve the internal logic so each class has a single, clear responsibility.

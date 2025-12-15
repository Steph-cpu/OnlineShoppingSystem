#include "ProductManager.h"
#include "ShoppingCart.h"
#include "User.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

void testAll() {
    ProductManager pm;

    cout << "\n=== Add products via addProduct (follow console prompts) ===" << endl;
    // 推荐输入序列见下方注释，确保覆盖有尺码和无尺码两种库存模式
    int idMenEast = pm.addProduct("MenShirt_Eastern", Category::Men, Section::Eastern, 199.0);
    // 输入建议：hasSize=1; XS=10 S=20 M=30 L=40 XL=50

    int idMenWest = pm.addProduct("MenBoots_Western", Category::Men, Section::Western, 499.0);
    // 输入建议：hasSize=0; total(None)=15

    int idWomenEast = pm.addProduct("WomenDress_Eastern", Category::Women, Section::Eastern, 299.0);
    // 输入建议：hasSize=1; XS=5 S=10 M=8 L=6 XL=3

    int idWomenOther = pm.addProduct("WomenAccessory_Other", Category::Women, Section::Other, 99.0);
    // 输入建议：hasSize=0; total(None)=100

    int idKidsBoys = pm.addProduct("KidsToy_Boys", Category::Kids, Section::Boys, 59.0);
    // 输入建议：hasSize=0; total(None)=200

    int idKidsGirls = pm.addProduct("KidsSkirt_Girls", Category::Kids, Section::Girls, 149.0);
    // 输入建议：hasSize=1; XS=2 S=3 M=4 L=2 XL=1

    int idOther = pm.addProduct("GiftCard", Category::Other, Section::Other, 50.0);
    // 输入建议：hasSize=0; total(None)=1000

    cout << "\n=== Display all products ===" << endl;
    pm.displayAllProducts();

    cout << "\n=== Display by category (Men) ===" << endl;
    pm.displayByCategory(Category::Men);

    cout << "\n=== Display by section (Women, Eastern) ===" << endl;
    pm.displayBySection(Category::Women, Section::Eastern);

    cout << "\n=== Display single product ===" << endl;
    pm.displaySingleProduct(idKidsGirls);

    cout << "\n=== Get productID by name ===" << endl;
    int idByName = pm.getProductID("GiftCard");
    cout << "ID for 'GiftCard': " << idByName << endl;

    cout << "\n=== Update product: price, stock, name, category/section ===" << endl;
    // 更新价格
    pm.updateProduct(idMenEast, 219.0);
    // 更新库存（尺码型）
    pm.updateProduct(idMenEast, Size::M, 35);
    // 更新名字（避免重名）
    pm.updateProduct(idWomenOther, string("WomenAccessory_Other_v2"));
    // 更新分类与分区：把 Boys 玩具移到 Other(必须与 Other/Other 对应)
    pm.updateProduct(idKidsBoys, Category::Other, Section::Other);

    cout << "\n=== Display after updates ===" << endl;
    pm.displayAllProducts();

    cout << "\n=== Save & reload products ===" << endl;
    const string productFile = "products_test.txt";
    pm.saveToFile(productFile);
    ProductManager pm2;
    pm2.loadFromFile(productFile);
    pm2.displayAllProducts();

    cout << "\n=== Remove a product ===" << endl;
    pm2.removeProduct(idMenWest);
    pm2.displayAllProducts();

    // 购物车测试（需要交互式输入）
    cout << "\n=== Shopping cart operations (follow console prompts) ===" << endl;
    ShoppingCart cart;

    cout << "\n--- Add items to cart ---" << endl;
    // 推荐：对有 None 库存的产品，系统会自动选择 Size::None
    // 对有尺码的产品，系统会让你输入尺码索引（0-4）
    cart.addItem(idMenEast, pm2);     // 输入建议：quantity=2，然后选择尺码，比如 2(M)
    cart.addItem(idKidsGirls, pm2);   // 输入建议：quantity=3，选择尺码，比如 1(S)
    cart.addItem(idOther, pm2);       // 输入建议：quantity=5（自动 None）

    cout << "\n--- Display cart ---" << endl;
    cart.displayCart(pm2);

    cout << "\n--- Update item in cart ---" << endl;
    cart.updateItem(idMenEast, pm2);  // 输入建议：quantity=4，选择尺码同上（覆盖原尺码数量）

    cout << "\n--- Remove one item ---" << endl;
    cart.removeItem(idOther);

    cout << "\n--- Calculate total ---" << endl;
    double total = cart.calculateTotal(pm2);
    cout << "Cart total: " << total << endl;

    cout << "\n--- Save & reload cart ---" << endl;
    string cartFile = ShoppingCart::getShoppingCartFileName(1001);
    cart.saveToFile(cartFile);
    ShoppingCart cart2;
    cart2.loadFromFile(cartFile);
    cart2.displayCart(pm2);

    cout << "\n=== Test finished ===" << endl;
}

int main() {
    testAll();
    return 0;
}

现有类：
product类：记录商品基本信息，包含3个枚举（size，category和section） // 我新增了size中的none（直接通过hassize来判断是不是具有size属性），category中的other和section中的other。
注意：当category为other时，section只能为other。其他的category另外包含other这个section，即每个category有3个section。size枚举中的none和size（xs，s，
m。。。）不能同时存在。product的name只能含有英文字母（不能有数字和逗号）

productmanager类：管理商品，对商品增删改查。购物车只是暂存用户的购买意图，不代表库存已经锁定
原理： vector<vector<unordered_map<int,Product>>> 记录所有商品，按层级分别是category，section，每个商品（productID到product的键值对）
unordered_map<int,int> map; 存储productID到category的键值对，方便快速找到商品所属类别
unordered_map<string,int> nameMap; 存储productName到productID的键值对，方便通过名称查找商品，并确保商品name的唯一性
功能：对所有商品的增删改查
1.添加商品：传入商品信息，生成唯一的productID，存储到相应的category和section（第一个vector）中，同时更新map和nameMap
2.删除商品：
3.更新商品info：多种函数重载，可以改变size的quantity，name，price，所属category，section的信息，函数内部会判断传入的信息是否合法（如name唯一性，category和section的对应关系等），并更新
4.查询商品：所有的display都只能传入productID作为parameter，但提供getName函数，你可以在user类中通过nameMap先找到productID再调用display函数
分为：显示单个/按section/按category/所有商品 display商品
5.读写文件：将所有商品信息写入文件，读取文件时清空现有商品信息，重新加载。
一个product manager只能有一个实例化对象，因此整个系统只有一个文件，每次login之后要实例化一个product manager对象出来，整个过程都对这个对象修改。

shoppingcart类：记录用户的购物车信息，一个用户对应一个购物车，一个购物车对应一个文件。要求用户买购物车内的所有商品，不能只买部分商品。
原理：unordered_map<int,vector<int>> key：productID value：下标为size枚举值的vector，记录每个size的quantity
功能：对购物车的增删改查
1.添加商品：只传入productID，函数内会让用户选择size和quantity，然后更新购物车信息
2.删除商品：只传入productID，函数就会删除所有对应product，不区分size
3.更新商品quantity：传入productID，系统会根据其有没有size属性让用户输入size（如有）和quantity，然后更新购物车信息
4.显示购物车信息：显示购物车内所有商品的详细信息，包括商品基本信息和购物车内的quantity，和总价
5.清空购物车
6.计算总价：返回购物车里的商品总价（double类型）
7.文件读写：提供getfilename函数：需要user传入一个userID（每个用户的userID一定要唯一），生成对应的filename。将每个user的购物车商品读写文件。

未实现的类：
user类： 为每个用户生成一个独一无二的userid，负责用户的注册/登陆，对购物车的控制，进行交易（根据用户的等级进行折扣，交易完更新用户的等级信息），以及查找每个用户的交易记录。管理员可以查看所有用户的信息。user类是总类，要调取productmanager类和shoppingcart类和transaction的功能。
user类的读写，只用一个文件，记录所有用户的信息（userid，username，password，level等）。每次登陆时读取文件，登出时写入文件。
transaction类： 为每次交易生成一个独一无二的transactionid（只需要对于单个用户是独一无二的），每次交易前检查购物车内商品的所对应库存够不够，如果不够让用户选择怎么办（删除或者更新商品数量），然后调用对应的函数，对新的购物车的获取金额并结算。每次交易结束后清空用户的购物车，及时更新商品的stock（product里面的update函数）每个用户的交易记录读写文件（每个用户一个文件，写一个生成filename的函数根据userid）。记录交易的所有商品信息，总价，折扣等。提供显示/查找交易记录的功能。
菜单函数： 提供用户交互界面，调用user类的功能。包括注册/登陆界面，用户主界面（查看商品，管理购物车，进行交易等），管理员主界面（管理商品，查看用户信息等）

优缺点：购物车使用hashmap，所以每次输出的商品顺序是不固定的，但查找和更新速度快。product manager在vector内部也是hashmap，所以在每个section
内的商品顺序也是不固定的，但查找和更新速度快。整体上，系统的查找和更新速度较快，但输出顺序不固定。
在类内函数直接写了cin cout。
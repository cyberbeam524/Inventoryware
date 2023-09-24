#include <cstdint>
#include <iostream>
#include <vector>
# include <fstream>  
# include <stdio.h>  
# include <string.h>  

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::impl::takes_array;
using bsoncxx::builder::basic::make_document;

using namespace std;  

class Inventory {
    // handles all mongodb application operations 
    // after inputs are taken from console presentation layer
  public:
    int speed(int maxSpeed);
    mongocxx::database db;
    mongocxx::collection collection;
    mongocxx::client client;
    // mongocxx::pool ownPool;

    Inventory(string dbName, string tableName) {
        mongocxx::instance instance{};
        mongocxx::uri uri("mongodb://localhost:27017");
        // mongocxx::uri uri("mongodb://localhost:27017?minPoolSize=3&maxPoolSize=3");
        mongocxx::client client(uri);
        this->client = std::move(client);
        this->db = this->client[dbName];
        this->collection = this->db[tableName];

        // this->ownPool = pool;
        // this->ownPool = std::move(newPool);
        // this->ownPool = std::move(newPool);
        // std::vector<std::thread> threads{};
    };

    ~Inventory(){ 
        // delete *this->collection;
        // delete *this->db;
        // delete *this->client;
        cout << " ----------------- Thank you for using the Inventory ----------------- \n~Closing program~\n";
    }

    void insert_inventory(string name, string expiryName, int quantity) {
        auto insert_one_result = this->collection.insert_one(
            make_document(kvp("Item Name", name),
                        kvp("Expiration Date", expiryName),
                        kvp("Quantity", quantity)));
        assert(insert_one_result);
        auto doc_id = insert_one_result->inserted_id();
        assert(doc_id.type() == bsoncxx::type::k_oid);
    };

    void print_inventory(){
        auto cursor_all = this->collection.find({});
        std::cout << "Collection '" << this->collection.name()
            << "' contains these documents:" << std::endl;
        for (auto doc : cursor_all) {
            std::cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << std::endl;
        }
        std::cout << std::endl;
    }

    void delete_single_item(string itemName){
        auto find_one_filtered_result = collection.find_one(make_document(kvp("Item Name", itemName)));
        if (find_one_filtered_result) {
            cout << "Deleted: ";
            std::cout << bsoncxx::to_json(*find_one_filtered_result, bsoncxx::ExtendedJsonMode::k_relaxed) << std::endl;
            auto delete_one_result = collection.delete_one(make_document(kvp("Item Name", itemName)));
            assert(delete_one_result);
            assert(delete_one_result->deleted_count() == 1);
        }else{
            cout << "This item cannot be found to be deleted";
        }
    }
};

void writeEntry(){
    char data [ 15 ] ;  
    ofstream outfile ;  
    outfile.open ( "./output/Example.txt" , ios::app ) ; 
    cout << "Enter input:" ;  
    cin >> data ;  
    outfile << data << " \t " ;  
    int len = strlen ( data ) ;   
    while ( len < 15 ) {  
        data [ len ] = ' ' ;  
        len = len + 1 ;  
    }
}

class ConnectionContainer{
    public:
    int available;
    // int idx;
    mongocxx::client cli;

    ConnectionContainer(){
        mongocxx::uri uri("mongodb://localhost:27017");
        mongocxx::client client(uri);
        this->cli = std::move(client);
        this->available = 0;
    };

    // ConnectionContainer& ConnectionContainer::operator=(const ConnectionContainer& rhs) {}
        
};


// todo: for massive insertion and deletion of documents 
// - use shared ptr from boost to custompool !!
class CustomPool{
    public:
    // vector<mongocxx::client> connectionsPool{};
    vector<ConnectionContainer> connectionsPool{};
    vector<int> tracker{};
    // mutable std::mutex                                               m_connections_mtx;
    // std::unordered_map< mongocxx::uri*, std::unique_ptr< mongocxx::uri > > m_connections_idle;
    // std::unordered_map< mongocxx::uri*, std::unique_ptr< mongocxx::uri > > m_connections_busy;
    CustomPool(int connections){
        mongocxx::uri uri("mongodb://localhost:27017");
        for (int i = 0; i < connections; i++){
            
            this->tracker.push_back(0);
            for (int e = 0; e < tracker.size(); e++){cout << "" << tracker[e] << ",";}
            // mongocxx::client client(uri);
            
            // this->connectionsPool.push_back(std::move(client));
            // cout << "the pool is this big: " << connectionsPool.size() << "\n";
            ConnectionContainer conn{};
            // this->connectionsPool.push_back(conn);
            cout << "the pool is this big: " << connectionsPool.size() << "\n";
            }
    };

    ~CustomPool(){
        for (int i = 0; i < connectionsPool.size(); i++){
            connectionsPool.pop_back();
        }
        cout << "\nClosed database connection pool " << connectionsPool.size()<< "\n";
        // delete connectionsPool;
    }
    
    // // todo: templating for 2 different return types since mongocxx::client cannot be nullptr
    void acquireClient() {}
};

void tryCustomPool(){
    string dbName = "mydb"; string tableName = "inventory"; int option = 0;
    CustomPool dbPool(4);
    {
        // auto conn = dbPool.acquireClient();
        // auto coll = conn.cli[dbName][tableName];

        // auto cursor_all = coll.find({});
        //     std::cout << "Collection '" << coll.name()
        //         << "' contains these documents:" << std::endl;
        //     for (auto doc : cursor_all) {
        //         std::cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << std::endl;
        //     }
        //     std::cout << std::endl;
    }
        // --------------------v-----

    // int idx2 = dbPool.acquireClient();
    // auto coll2 = dbPool.connectionsPool[idx2][dbName][tableName];

    // auto cursor_all2 = coll2.find({});
    //     std::cout << "Collection '" << coll2.name()
    //         << "' contains these documents:" << std::endl;
    //     for (auto doc : cursor_all2) {
    //         std::cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << std::endl;
    //     }
    //     std::cout << std::endl;
}




int main()
{
    string dbName = "mydb"; string tableName = "inventory"; int option = 0;


    // Inventory object to handle application layer operations
    Inventory inventory(dbName, tableName);
    // mongodb://localhost:27017?minPoolSize=3&maxPoolSize=3
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::pool ownPool{uri};
    

    // // CustomPool implementation attempt:
    // // with existing mongodb client
    // // - not entirely possible due to internal destruction of client when moved aroudnd
    // tryCustomPool();

    while (option != 6 ) {  
    // This prints out all the available options in the DB
    cout << " \n Available operations: \n1. Add New "  
            "Inventory Item \n2. "  
        << "Display All Inventory Records \n3. Delete Inventory Item"
        << "\n6. Exit"
        << "\nEnter option: " ;  
    cin >> option ;  

    while(!cin){
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Please re-enter a valid option: "; 
        cin >> option;
    }

    if (option == 1) {
        {
            string name; string expiryName; int quantity;
            cout << "Item Name: ";
            cin >> name ;  
            cout << "Quantity: "; 
            cin >> quantity;
            while(!cin){
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "Please re-enter quantity: "; 
                cin >> quantity;
            }
            cout << "Expiration Date: ";  
            cin >> expiryName ;  

            inventory.insert_inventory(name, expiryName, quantity);
        }
    }else if(option == 2){
        //  Print All Documents in a Collection
        {
            // // @todo:sort by columns?
            // // @todo:print the first few of the resulting dataframe?
            inventory.print_inventory();
        }
    }else if(option == 3){
        // // Delete a Single Document
        {
            string itemName;
            cout << "Item Name to delete:";  
            cin >> itemName ; 
            inventory.delete_single_item(itemName);
        }
    }else if(option == 4){
        // multithreaded insertion of documents
        // independent processes as they dont depend on one another - detach to become daemon processes
        // or have a pool of threads since its expensive 
        // to create threads so reuse existing threads
        // // todo:add list to text document for users to print
        {
            // // writeEntry();
        }
    }
    }

    // delete *inventory;
    return 0;
}

// @todo: need totemplate function for comparing objectss with same parent -- only way of polymorphism for mehtods
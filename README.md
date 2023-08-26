# Inventoryware

Console Program Demo:
<img src="./images/console_example.gif" style="width:100%">

### Start Console Program:
To start the program, open terminal and run 
``` make mango ``` 
or 
```
g++ --std=c++11 -o ./output/mongo mongo.cpp -I/usr/local/include/mongocxx/v_noabi \
	-I/usr/local/include/bsoncxx/v_noabi \
	-L/usr/local/lib -lmongocxx -lbsoncxx
	./output/mongo
```

<img src="./images/startprogram.png" style="width:100%">


### Add Item to Inventory:
Type option 1 to add items. Users will be prompted for the name, quantity, and expiration date of the items to be added.

<img src="./images/additem.png" style="width:100%">

### Remove Item from Inventory:
Type option 3 to add items. Users will be prompted for the name of the item to be removed.

<img src="./images/removeitem.png" style="width:100%">

### View Items in Inventory:
Type option 2 to view items current items in inventory as shown above.

<img src="./images/removeitem.png" style="width:100%">

Inventory Layered Architecture:

<img src="./images/architecture_layers.png" style="width:100%">

## To Dos:
- [x] Basic CRUD Operations
- [ ] Inventory Items abstraction for better comparison between items
- [ ] Sorting inventory items
    - [ ] by expiry date
        - [ ] option to remove items with expiry date more than today
    - [ ] other columns 
    
References:
- https://github.com/mongodb/mongo-cxx-driver
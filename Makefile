mongo:
	g++ --std=c++11 -o ./output/mongo mongo.cpp -I/usr/local/include/mongocxx/v_noabi \
	-I/usr/local/include/bsoncxx/v_noabi \
	-L/usr/local/lib -lmongocxx -lbsoncxx
	./output/mongo

postgres_threadpool:
	g++ postgres_threadpool.cpp -o ./output/test.o --std=c++17 \
	-I/usr/local/include/mongocxx/v_noabi \
	-I/usr/local/include/bsoncxx/v_noabi \
	-L/usr/local/lib -lmongocxx -lbsoncxx \
	-I/opt/homebrew/Cellar/glew/2.2.0_1 \
	-I/usr/local/include \
	-lpqxx -lpq \
	-I/opt/homebrew/opt/libpq/include -I/opt/homebrew/opt/libpqxx/include \
	-L/opt/homebrew/opt/libpq/lib -L/opt/homebrew/opt/libpqxx/lib
	./output/test.o

all:server.exe online offline
server.exe: bin/TestEchoserver.o bin/TcpServer.o bin/Acceptor.o bin/EchoServer.o bin/EventLoop.o bin/InetAddress.o bin/Socket.o bin/SocketIO.o bin/TaskQueue.o bin/TcpConnection.o bin/ThreadPool.o bin/HTTPRequest.o bin/ProcessFile.o bin/HTTPResponse.o bin/HTTPRequestHandler.o bin/Dictonary.o bin/RSS.o bin/tinyxml2.o bin/webPage.o bin/webPageHandler.o bin/RedisSingleton.o bin/DoubleLruCache.o
	g++ bin/TestEchoserver.o bin/TcpServer.o bin/Acceptor.o bin/EchoServer.o bin/EventLoop.o bin/InetAddress.o bin/Socket.o bin/SocketIO.o bin/TaskQueue.o bin/TcpConnection.o bin/ThreadPool.o bin/HTTPRequest.o bin/ProcessFile.o bin/HTTPResponse.o bin/HTTPRequestHandler.o bin/Dictonary.o bin/RSS.o bin/tinyxml2.o bin/webPage.o bin/webPageHandler.o bin/RedisSingleton.o bin/DoubleLruCache.o -o server.exe -lhiredis -lpthread

online: bin/OnlineServer.o bin/TcpServer.o bin/Acceptor.o bin/EchoServer.o bin/EventLoop.o bin/InetAddress.o bin/Socket.o bin/SocketIO.o bin/TaskQueue.o bin/TcpConnection.o bin/ThreadPool.o bin/HTTPRequest.o bin/ProcessFile.o bin/HTTPResponse.o bin/HTTPRequestHandler.o bin/Dictonary.o bin/RSS.o bin/tinyxml2.o bin/webPage.o bin/webPageHandler.o bin/RedisSingleton.o bin/DoubleLruCache.o
	g++ bin/OnlineServer.o bin/TcpServer.o bin/Acceptor.o bin/EchoServer.o bin/EventLoop.o bin/InetAddress.o bin/Socket.o bin/SocketIO.o bin/TaskQueue.o bin/TcpConnection.o bin/ThreadPool.o bin/HTTPRequest.o bin/ProcessFile.o bin/HTTPResponse.o bin/HTTPRequestHandler.o bin/Dictonary.o bin/RSS.o bin/tinyxml2.o bin/webPage.o bin/webPageHandler.o bin/RedisSingleton.o bin/DoubleLruCache.o -o bin/online -lhiredis -lpthread

offline: bin/OfflinePreprocess.o bin/TcpServer.o bin/Acceptor.o bin/EchoServer.o bin/EventLoop.o bin/InetAddress.o bin/Socket.o bin/SocketIO.o bin/TaskQueue.o bin/TcpConnection.o bin/ThreadPool.o bin/HTTPRequest.o bin/ProcessFile.o bin/HTTPResponse.o bin/HTTPRequestHandler.o bin/Dictonary.o bin/RSS.o bin/tinyxml2.o bin/webPage.o bin/webPageHandler.o bin/RedisSingleton.o bin/DoubleLruCache.o
	g++ bin/OfflinePreprocess.o bin/TcpServer.o bin/Acceptor.o bin/EchoServer.o bin/EventLoop.o bin/InetAddress.o bin/Socket.o bin/SocketIO.o bin/TaskQueue.o bin/TcpConnection.o bin/ThreadPool.o bin/HTTPRequest.o bin/ProcessFile.o bin/HTTPResponse.o bin/HTTPRequestHandler.o bin/Dictonary.o bin/RSS.o bin/tinyxml2.o bin/webPage.o bin/webPageHandler.o bin/RedisSingleton.o bin/DoubleLruCache.o -o bin/offline -lhiredis -lpthread

bin/ThreadPool.o:src/ThreadPool.cc include/ThreadPool.h
	g++ -c src/ThreadPool.cc -o bin/ThreadPool.o -g -I include
bin/TestEchoserver.o: src/TestEchoserver.cc
	g++ -c src/TestEchoserver.cc -o bin/TestEchoserver.o -g -I include
bin/TcpServer.o: src/TcpServer.cc include/TcpServer.h
	g++ -c src/TcpServer.cc -o bin/TcpServer.o -g -I include
bin/Acceptor.o: src/Acceptor.cc include/Acceptor.h
	g++ -c src/Acceptor.cc -o bin/Acceptor.o -g -I include
bin/EchoServer.o: src/EchoServer.cc include/Echoserver.h
	g++ -c src/EchoServer.cc -o bin/EchoServer.o -g -I include
bin/EventLoop.o: src/EventLoop.cc include/EventLoop.h
	g++ -c src/EventLoop.cc -o bin/EventLoop.o -g -I include
bin/InetAddress.o: src/InetAddress.cc include/InetAddress.h
	g++ -c src/InetAddress.cc -o bin/InetAddress.o -g -I include
bin/Socket.o: src/Socket.cc include/Socket.h
	g++ -c src/Socket.cc -o bin/Socket.o -g -I include
bin/SocketIO.o: src/SocketIO.cc include/SocketIO.h
	g++ -c src/SocketIO.cc -o bin/SocketIO.o -g -I include
bin/TaskQueue.o: src/TaskQueue.cc include/TaskQueue.h
	g++ -c src/TaskQueue.cc -o bin/TaskQueue.o -g -I include
bin/TcpConnection.o: src/TcpConnection.cc include/TcpConnection.h
	g++ -c src/TcpConnection.cc -o bin/TcpConnection.o -g -I include
bin/HTTPRequest.o: src/HTTPRequest.cc include/HTTPRequest.h
	g++ -c src/HTTPRequest.cc -o bin/HTTPRequest.o -g -I include
bin/ProcessFile.o: src/ProcessFile.cc include/ProcessFile.h
	g++ -c src/ProcessFile.cc -o bin/ProcessFile.o -g -I include
bin/HTTPResponse.o: src/HTTPResponse.cc include/HTTPResponse.h
	g++ -c src/HTTPResponse.cc -o bin/HTTPResponse.o -g -I include
bin/HTTPRequestHandler.o: src/HTTPRequestHandler.cc include/HTTPRequestHandler.h
	g++ -c src/HTTPRequestHandler.cc -o bin/HTTPRequestHandler.o -g -I include
bin/Dictonary.o: src/Dictonary.cc include/Dictonary.h
	g++ -c src/Dictonary.cc -o bin/Dictonary.o -g -I include
bin/RSS.o: src/RSS.cc include/RSS.h
	g++ -c src/RSS.cc -o bin/RSS.o -g -I include
bin/tinyxml2.o: src/tinyxml2.cpp include/tinyxml2.h
	g++ -c src/tinyxml2.cpp -o bin/tinyxml2.o -g -I include
bin/webPage.o: src/webPage.cc include/webPage.h
	g++ -c src/webPage.cc -o bin/webPage.o -g -I include
bin/webPageHandler.o: src/webPageHandler.cc include/webPageHandler.h
	g++ -c src/webPageHandler.cc -o bin/webPageHandler.o -g -I include
bin/RedisSingleton.o: src/RedisSingleton.cc include/RedisSingleton.h
	g++ -c src/RedisSingleton.cc -o bin/RedisSingleton.o -g -I include
bin/DoubleLruCache.o: src/DoubleLruCache.cc include/DoubleLruCache.h
	g++ -c src/DoubleLruCache.cc -o bin/DoubleLruCache.o -g -I include -I ./simhash-1.2.1/include
bin/OnlineServer.o: src/OnlineServer.cc
	g++ -c src/OnlineServer.cc -o bin/OnlineServer.o -g -I include
bin/OfflinePreprocess.o: src/OfflinePreprocess.cc
	g++ -c src/OfflinePreprocess.cc -o bin/OfflinePreprocess.o -g -I include

clean:
	rm -f bin/*.o server.exe bin/online bin/offline

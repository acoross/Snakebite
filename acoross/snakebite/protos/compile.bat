echo "compile start"

<<<<<<< HEAD
protoc.exe -I ./ --grpc_out=./ --plugin=protoc-gen-grpc="acoross_cpp_plugin.exe" snakebite_message.proto
=======
protoc.exe -I ./ --grpc_out=./ --plugin=protoc-gen-grpc="cpp_plugin.exe" snakebite_message.proto
>>>>>>> origin/master
protoc.exe -I ./ --cpp_out=./ snakebite_message.proto

pause
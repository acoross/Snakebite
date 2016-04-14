echo "compile start"

protoc.exe -I ./ --grpc_out=./ --plugin=protoc-gen-grpc="acoross_cpp_plugin.exe" test_rpc.proto
protoc.exe -I ./ --cpp_out=./ test_rpc.proto

pause
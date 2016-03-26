echo "compile start"

protoc.exe -I ./ --cpp_out=./ snakebite_message.proto
protoc.exe -I ./ --cpp_out=./ sc_snakebite_message.proto

pause
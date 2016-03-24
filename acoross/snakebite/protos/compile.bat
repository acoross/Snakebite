echo "compile start"

protoc.exe -I ./ --cpp_out=./ snakebite_message.proto
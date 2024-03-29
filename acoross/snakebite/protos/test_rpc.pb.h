// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: test_rpc.proto

#ifndef PROTOBUF_test_5frpc_2eproto__INCLUDED
#define PROTOBUF_test_5frpc_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace acoross {
namespace rpc_test {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_test_5frpc_2eproto();
void protobuf_AssignDesc_test_5frpc_2eproto();
void protobuf_ShutdownFile_test_5frpc_2eproto();

class HelloReply;
class HelloRequest;
class InitPlayerSnakeRequest;
class TurnKeyDownRequest;
class TurnKeyUpRequest;
class VoidReply;

// ===================================================================

class HelloRequest : public ::google::protobuf::Message
{
public:
	HelloRequest();
	virtual ~HelloRequest();

	HelloRequest(const HelloRequest& from);

	inline HelloRequest& operator=(const HelloRequest& from)
	{
		CopyFrom(from);
		return *this;
	}

	static const ::google::protobuf::Descriptor* descriptor();
	static const HelloRequest& default_instance();

	void Swap(HelloRequest* other);

	// implements Message ----------------------------------------------

	inline HelloRequest* New() const { return New(NULL); }

	HelloRequest* New(::google::protobuf::Arena* arena) const;
	void CopyFrom(const ::google::protobuf::Message& from);
	void MergeFrom(const ::google::protobuf::Message& from);
	void CopyFrom(const HelloRequest& from);
	void MergeFrom(const HelloRequest& from);
	void Clear();
	bool IsInitialized() const;

	int ByteSize() const;
	bool MergePartialFromCodedStream(
		::google::protobuf::io::CodedInputStream* input);
	void SerializeWithCachedSizes(
		::google::protobuf::io::CodedOutputStream* output) const;
	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
	int GetCachedSize() const { return _cached_size_; }
private:
	void SharedCtor();
	void SharedDtor();
	void SetCachedSize(int size) const;
	void InternalSwap(HelloRequest* other);
private:
	inline ::google::protobuf::Arena* GetArenaNoVirtual() const
	{
		return _internal_metadata_.arena();
	}
	inline void* MaybeArenaPtr() const
	{
		return _internal_metadata_.raw_arena_ptr();
	}
public:

	::google::protobuf::Metadata GetMetadata() const;

	// nested types ----------------------------------------------------

	// accessors -------------------------------------------------------

	// optional string name = 1;
	void clear_name();
	static const int kNameFieldNumber = 1;
	const ::std::string& name() const;
	void set_name(const ::std::string& value);
	void set_name(const char* value);
	void set_name(const char* value, size_t size);
	::std::string* mutable_name();
	::std::string* release_name();
	void set_allocated_name(::std::string* name);

	// @@protoc_insertion_point(class_scope:acoross.rpc_test.HelloRequest)
private:

	::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
	bool _is_default_instance_;
	::google::protobuf::internal::ArenaStringPtr name_;
	mutable int _cached_size_;
	friend void  protobuf_AddDesc_test_5frpc_2eproto();
	friend void protobuf_AssignDesc_test_5frpc_2eproto();
	friend void protobuf_ShutdownFile_test_5frpc_2eproto();

	void InitAsDefaultInstance();
	static HelloRequest* default_instance_;
};
// -------------------------------------------------------------------

class HelloReply : public ::google::protobuf::Message
{
public:
	HelloReply();
	virtual ~HelloReply();

	HelloReply(const HelloReply& from);

	inline HelloReply& operator=(const HelloReply& from)
	{
		CopyFrom(from);
		return *this;
	}

	static const ::google::protobuf::Descriptor* descriptor();
	static const HelloReply& default_instance();

	void Swap(HelloReply* other);

	// implements Message ----------------------------------------------

	inline HelloReply* New() const { return New(NULL); }

	HelloReply* New(::google::protobuf::Arena* arena) const;
	void CopyFrom(const ::google::protobuf::Message& from);
	void MergeFrom(const ::google::protobuf::Message& from);
	void CopyFrom(const HelloReply& from);
	void MergeFrom(const HelloReply& from);
	void Clear();
	bool IsInitialized() const;

	int ByteSize() const;
	bool MergePartialFromCodedStream(
		::google::protobuf::io::CodedInputStream* input);
	void SerializeWithCachedSizes(
		::google::protobuf::io::CodedOutputStream* output) const;
	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
	int GetCachedSize() const { return _cached_size_; }
private:
	void SharedCtor();
	void SharedDtor();
	void SetCachedSize(int size) const;
	void InternalSwap(HelloReply* other);
private:
	inline ::google::protobuf::Arena* GetArenaNoVirtual() const
	{
		return _internal_metadata_.arena();
	}
	inline void* MaybeArenaPtr() const
	{
		return _internal_metadata_.raw_arena_ptr();
	}
public:

	::google::protobuf::Metadata GetMetadata() const;

	// nested types ----------------------------------------------------

	// accessors -------------------------------------------------------

	// optional string message = 1;
	void clear_message();
	static const int kMessageFieldNumber = 1;
	const ::std::string& message() const;
	void set_message(const ::std::string& value);
	void set_message(const char* value);
	void set_message(const char* value, size_t size);
	::std::string* mutable_message();
	::std::string* release_message();
	void set_allocated_message(::std::string* message);

	// @@protoc_insertion_point(class_scope:acoross.rpc_test.HelloReply)
private:

	::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
	bool _is_default_instance_;
	::google::protobuf::internal::ArenaStringPtr message_;
	mutable int _cached_size_;
	friend void  protobuf_AddDesc_test_5frpc_2eproto();
	friend void protobuf_AssignDesc_test_5frpc_2eproto();
	friend void protobuf_ShutdownFile_test_5frpc_2eproto();

	void InitAsDefaultInstance();
	static HelloReply* default_instance_;
};
// -------------------------------------------------------------------

class VoidReply : public ::google::protobuf::Message
{
public:
	VoidReply();
	virtual ~VoidReply();

	VoidReply(const VoidReply& from);

	inline VoidReply& operator=(const VoidReply& from)
	{
		CopyFrom(from);
		return *this;
	}

	static const ::google::protobuf::Descriptor* descriptor();
	static const VoidReply& default_instance();

	void Swap(VoidReply* other);

	// implements Message ----------------------------------------------

	inline VoidReply* New() const { return New(NULL); }

	VoidReply* New(::google::protobuf::Arena* arena) const;
	void CopyFrom(const ::google::protobuf::Message& from);
	void MergeFrom(const ::google::protobuf::Message& from);
	void CopyFrom(const VoidReply& from);
	void MergeFrom(const VoidReply& from);
	void Clear();
	bool IsInitialized() const;

	int ByteSize() const;
	bool MergePartialFromCodedStream(
		::google::protobuf::io::CodedInputStream* input);
	void SerializeWithCachedSizes(
		::google::protobuf::io::CodedOutputStream* output) const;
	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
	int GetCachedSize() const { return _cached_size_; }
private:
	void SharedCtor();
	void SharedDtor();
	void SetCachedSize(int size) const;
	void InternalSwap(VoidReply* other);
private:
	inline ::google::protobuf::Arena* GetArenaNoVirtual() const
	{
		return _internal_metadata_.arena();
	}
	inline void* MaybeArenaPtr() const
	{
		return _internal_metadata_.raw_arena_ptr();
	}
public:

	::google::protobuf::Metadata GetMetadata() const;

	// nested types ----------------------------------------------------

	// accessors -------------------------------------------------------

	// optional int32 err = 1;
	void clear_err();
	static const int kErrFieldNumber = 1;
	::google::protobuf::int32 err() const;
	void set_err(::google::protobuf::int32 value);

	// @@protoc_insertion_point(class_scope:acoross.rpc_test.VoidReply)
private:

	::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
	bool _is_default_instance_;
	::google::protobuf::int32 err_;
	mutable int _cached_size_;
	friend void  protobuf_AddDesc_test_5frpc_2eproto();
	friend void protobuf_AssignDesc_test_5frpc_2eproto();
	friend void protobuf_ShutdownFile_test_5frpc_2eproto();

	void InitAsDefaultInstance();
	static VoidReply* default_instance_;
};
// -------------------------------------------------------------------

class InitPlayerSnakeRequest : public ::google::protobuf::Message
{
public:
	InitPlayerSnakeRequest();
	virtual ~InitPlayerSnakeRequest();

	InitPlayerSnakeRequest(const InitPlayerSnakeRequest& from);

	inline InitPlayerSnakeRequest& operator=(const InitPlayerSnakeRequest& from)
	{
		CopyFrom(from);
		return *this;
	}

	static const ::google::protobuf::Descriptor* descriptor();
	static const InitPlayerSnakeRequest& default_instance();

	void Swap(InitPlayerSnakeRequest* other);

	// implements Message ----------------------------------------------

	inline InitPlayerSnakeRequest* New() const { return New(NULL); }

	InitPlayerSnakeRequest* New(::google::protobuf::Arena* arena) const;
	void CopyFrom(const ::google::protobuf::Message& from);
	void MergeFrom(const ::google::protobuf::Message& from);
	void CopyFrom(const InitPlayerSnakeRequest& from);
	void MergeFrom(const InitPlayerSnakeRequest& from);
	void Clear();
	bool IsInitialized() const;

	int ByteSize() const;
	bool MergePartialFromCodedStream(
		::google::protobuf::io::CodedInputStream* input);
	void SerializeWithCachedSizes(
		::google::protobuf::io::CodedOutputStream* output) const;
	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
	int GetCachedSize() const { return _cached_size_; }
private:
	void SharedCtor();
	void SharedDtor();
	void SetCachedSize(int size) const;
	void InternalSwap(InitPlayerSnakeRequest* other);
private:
	inline ::google::protobuf::Arena* GetArenaNoVirtual() const
	{
		return _internal_metadata_.arena();
	}
	inline void* MaybeArenaPtr() const
	{
		return _internal_metadata_.raw_arena_ptr();
	}
public:

	::google::protobuf::Metadata GetMetadata() const;

	// nested types ----------------------------------------------------

	// accessors -------------------------------------------------------

	// optional string name = 1;
	void clear_name();
	static const int kNameFieldNumber = 1;
	const ::std::string& name() const;
	void set_name(const ::std::string& value);
	void set_name(const char* value);
	void set_name(const char* value, size_t size);
	::std::string* mutable_name();
	::std::string* release_name();
	void set_allocated_name(::std::string* name);

	// @@protoc_insertion_point(class_scope:acoross.rpc_test.InitPlayerSnakeRequest)
private:

	::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
	bool _is_default_instance_;
	::google::protobuf::internal::ArenaStringPtr name_;
	mutable int _cached_size_;
	friend void  protobuf_AddDesc_test_5frpc_2eproto();
	friend void protobuf_AssignDesc_test_5frpc_2eproto();
	friend void protobuf_ShutdownFile_test_5frpc_2eproto();

	void InitAsDefaultInstance();
	static InitPlayerSnakeRequest* default_instance_;
};
// -------------------------------------------------------------------

class TurnKeyDownRequest : public ::google::protobuf::Message
{
public:
	TurnKeyDownRequest();
	virtual ~TurnKeyDownRequest();

	TurnKeyDownRequest(const TurnKeyDownRequest& from);

	inline TurnKeyDownRequest& operator=(const TurnKeyDownRequest& from)
	{
		CopyFrom(from);
		return *this;
	}

	static const ::google::protobuf::Descriptor* descriptor();
	static const TurnKeyDownRequest& default_instance();

	void Swap(TurnKeyDownRequest* other);

	// implements Message ----------------------------------------------

	inline TurnKeyDownRequest* New() const { return New(NULL); }

	TurnKeyDownRequest* New(::google::protobuf::Arena* arena) const;
	void CopyFrom(const ::google::protobuf::Message& from);
	void MergeFrom(const ::google::protobuf::Message& from);
	void CopyFrom(const TurnKeyDownRequest& from);
	void MergeFrom(const TurnKeyDownRequest& from);
	void Clear();
	bool IsInitialized() const;

	int ByteSize() const;
	bool MergePartialFromCodedStream(
		::google::protobuf::io::CodedInputStream* input);
	void SerializeWithCachedSizes(
		::google::protobuf::io::CodedOutputStream* output) const;
	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
	int GetCachedSize() const { return _cached_size_; }
private:
	void SharedCtor();
	void SharedDtor();
	void SetCachedSize(int size) const;
	void InternalSwap(TurnKeyDownRequest* other);
private:
	inline ::google::protobuf::Arena* GetArenaNoVirtual() const
	{
		return _internal_metadata_.arena();
	}
	inline void* MaybeArenaPtr() const
	{
		return _internal_metadata_.raw_arena_ptr();
	}
public:

	::google::protobuf::Metadata GetMetadata() const;

	// nested types ----------------------------------------------------

	// accessors -------------------------------------------------------

	// optional int32 key = 1;
	void clear_key();
	static const int kKeyFieldNumber = 1;
	::google::protobuf::int32 key() const;
	void set_key(::google::protobuf::int32 value);

	// @@protoc_insertion_point(class_scope:acoross.rpc_test.TurnKeyDownRequest)
private:

	::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
	bool _is_default_instance_;
	::google::protobuf::int32 key_;
	mutable int _cached_size_;
	friend void  protobuf_AddDesc_test_5frpc_2eproto();
	friend void protobuf_AssignDesc_test_5frpc_2eproto();
	friend void protobuf_ShutdownFile_test_5frpc_2eproto();

	void InitAsDefaultInstance();
	static TurnKeyDownRequest* default_instance_;
};
// -------------------------------------------------------------------

class TurnKeyUpRequest : public ::google::protobuf::Message
{
public:
	TurnKeyUpRequest();
	virtual ~TurnKeyUpRequest();

	TurnKeyUpRequest(const TurnKeyUpRequest& from);

	inline TurnKeyUpRequest& operator=(const TurnKeyUpRequest& from)
	{
		CopyFrom(from);
		return *this;
	}

	static const ::google::protobuf::Descriptor* descriptor();
	static const TurnKeyUpRequest& default_instance();

	void Swap(TurnKeyUpRequest* other);

	// implements Message ----------------------------------------------

	inline TurnKeyUpRequest* New() const { return New(NULL); }

	TurnKeyUpRequest* New(::google::protobuf::Arena* arena) const;
	void CopyFrom(const ::google::protobuf::Message& from);
	void MergeFrom(const ::google::protobuf::Message& from);
	void CopyFrom(const TurnKeyUpRequest& from);
	void MergeFrom(const TurnKeyUpRequest& from);
	void Clear();
	bool IsInitialized() const;

	int ByteSize() const;
	bool MergePartialFromCodedStream(
		::google::protobuf::io::CodedInputStream* input);
	void SerializeWithCachedSizes(
		::google::protobuf::io::CodedOutputStream* output) const;
	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
	int GetCachedSize() const { return _cached_size_; }
private:
	void SharedCtor();
	void SharedDtor();
	void SetCachedSize(int size) const;
	void InternalSwap(TurnKeyUpRequest* other);
private:
	inline ::google::protobuf::Arena* GetArenaNoVirtual() const
	{
		return _internal_metadata_.arena();
	}
	inline void* MaybeArenaPtr() const
	{
		return _internal_metadata_.raw_arena_ptr();
	}
public:

	::google::protobuf::Metadata GetMetadata() const;

	// nested types ----------------------------------------------------

	// accessors -------------------------------------------------------

	// optional int32 key = 1;
	void clear_key();
	static const int kKeyFieldNumber = 1;
	::google::protobuf::int32 key() const;
	void set_key(::google::protobuf::int32 value);

	// @@protoc_insertion_point(class_scope:acoross.rpc_test.TurnKeyUpRequest)
private:

	::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
	bool _is_default_instance_;
	::google::protobuf::int32 key_;
	mutable int _cached_size_;
	friend void  protobuf_AddDesc_test_5frpc_2eproto();
	friend void protobuf_AssignDesc_test_5frpc_2eproto();
	friend void protobuf_ShutdownFile_test_5frpc_2eproto();

	void InitAsDefaultInstance();
	static TurnKeyUpRequest* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// HelloRequest

// optional string name = 1;
inline void HelloRequest::clear_name()
{
	name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& HelloRequest::name() const
{
	// @@protoc_insertion_point(field_get:acoross.rpc_test.HelloRequest.name)
	return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void HelloRequest::set_name(const ::std::string& value)
{

	name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
	// @@protoc_insertion_point(field_set:acoross.rpc_test.HelloRequest.name)
}
inline void HelloRequest::set_name(const char* value)
{

	name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
	// @@protoc_insertion_point(field_set_char:acoross.rpc_test.HelloRequest.name)
}
inline void HelloRequest::set_name(const char* value, size_t size)
{

	name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
		::std::string(reinterpret_cast<const char*>(value), size));
	// @@protoc_insertion_point(field_set_pointer:acoross.rpc_test.HelloRequest.name)
}
inline ::std::string* HelloRequest::mutable_name()
{

	// @@protoc_insertion_point(field_mutable:acoross.rpc_test.HelloRequest.name)
	return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* HelloRequest::release_name()
{

	return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void HelloRequest::set_allocated_name(::std::string* name)
{
	if (name != NULL)
	{

	}
	else
	{

	}
	name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
	// @@protoc_insertion_point(field_set_allocated:acoross.rpc_test.HelloRequest.name)
}

// -------------------------------------------------------------------

// HelloReply

// optional string message = 1;
inline void HelloReply::clear_message()
{
	message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& HelloReply::message() const
{
	// @@protoc_insertion_point(field_get:acoross.rpc_test.HelloReply.message)
	return message_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void HelloReply::set_message(const ::std::string& value)
{

	message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
	// @@protoc_insertion_point(field_set:acoross.rpc_test.HelloReply.message)
}
inline void HelloReply::set_message(const char* value)
{

	message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
	// @@protoc_insertion_point(field_set_char:acoross.rpc_test.HelloReply.message)
}
inline void HelloReply::set_message(const char* value, size_t size)
{

	message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
		::std::string(reinterpret_cast<const char*>(value), size));
	// @@protoc_insertion_point(field_set_pointer:acoross.rpc_test.HelloReply.message)
}
inline ::std::string* HelloReply::mutable_message()
{

	// @@protoc_insertion_point(field_mutable:acoross.rpc_test.HelloReply.message)
	return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* HelloReply::release_message()
{

	return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void HelloReply::set_allocated_message(::std::string* message)
{
	if (message != NULL)
	{

	}
	else
	{

	}
	message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
	// @@protoc_insertion_point(field_set_allocated:acoross.rpc_test.HelloReply.message)
}

// -------------------------------------------------------------------

// VoidReply

// optional int32 err = 1;
inline void VoidReply::clear_err()
{
	err_ = 0;
}
inline ::google::protobuf::int32 VoidReply::err() const
{
	// @@protoc_insertion_point(field_get:acoross.rpc_test.VoidReply.err)
	return err_;
}
inline void VoidReply::set_err(::google::protobuf::int32 value)
{

	err_ = value;
	// @@protoc_insertion_point(field_set:acoross.rpc_test.VoidReply.err)
}

// -------------------------------------------------------------------

// InitPlayerSnakeRequest

// optional string name = 1;
inline void InitPlayerSnakeRequest::clear_name()
{
	name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& InitPlayerSnakeRequest::name() const
{
	// @@protoc_insertion_point(field_get:acoross.rpc_test.InitPlayerSnakeRequest.name)
	return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void InitPlayerSnakeRequest::set_name(const ::std::string& value)
{

	name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
	// @@protoc_insertion_point(field_set:acoross.rpc_test.InitPlayerSnakeRequest.name)
}
inline void InitPlayerSnakeRequest::set_name(const char* value)
{

	name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
	// @@protoc_insertion_point(field_set_char:acoross.rpc_test.InitPlayerSnakeRequest.name)
}
inline void InitPlayerSnakeRequest::set_name(const char* value, size_t size)
{

	name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
		::std::string(reinterpret_cast<const char*>(value), size));
	// @@protoc_insertion_point(field_set_pointer:acoross.rpc_test.InitPlayerSnakeRequest.name)
}
inline ::std::string* InitPlayerSnakeRequest::mutable_name()
{

	// @@protoc_insertion_point(field_mutable:acoross.rpc_test.InitPlayerSnakeRequest.name)
	return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* InitPlayerSnakeRequest::release_name()
{

	return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void InitPlayerSnakeRequest::set_allocated_name(::std::string* name)
{
	if (name != NULL)
	{

	}
	else
	{

	}
	name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
	// @@protoc_insertion_point(field_set_allocated:acoross.rpc_test.InitPlayerSnakeRequest.name)
}

// -------------------------------------------------------------------

// TurnKeyDownRequest

// optional int32 key = 1;
inline void TurnKeyDownRequest::clear_key()
{
	key_ = 0;
}
inline ::google::protobuf::int32 TurnKeyDownRequest::key() const
{
	// @@protoc_insertion_point(field_get:acoross.rpc_test.TurnKeyDownRequest.key)
	return key_;
}
inline void TurnKeyDownRequest::set_key(::google::protobuf::int32 value)
{

	key_ = value;
	// @@protoc_insertion_point(field_set:acoross.rpc_test.TurnKeyDownRequest.key)
}

// -------------------------------------------------------------------

// TurnKeyUpRequest

// optional int32 key = 1;
inline void TurnKeyUpRequest::clear_key()
{
	key_ = 0;
}
inline ::google::protobuf::int32 TurnKeyUpRequest::key() const
{
	// @@protoc_insertion_point(field_get:acoross.rpc_test.TurnKeyUpRequest.key)
	return key_;
}
inline void TurnKeyUpRequest::set_key(::google::protobuf::int32 value)
{

	key_ = value;
	// @@protoc_insertion_point(field_set:acoross.rpc_test.TurnKeyUpRequest.key)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace rpc_test
}  // namespace acoross

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_test_5frpc_2eproto__INCLUDED

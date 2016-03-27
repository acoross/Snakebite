#ifndef SNAKEBITE_SNAKEBITE_MESSAGE_TYPE_H_
#define SNAKEBITE_SNAKEBITE_MESSAGE_TYPE_H_

namespace acoross {
namespace snakebite {

enum class SnakebiteMessageType : unsigned short
{
	None = 0,
	InitPlayerSnake,
	TurnKeyDown,
	TurnKeyUp,
	Max
};

enum class SC_SnakebiteMessageType : unsigned short
{
	None = 0,
	ReplyInitPlayerSnake,
	UpdateGameObjects,
	Max
};

}
}
#endif //SNAKEBITE_SNAKEBITE_MESSAGE_TYPE_H_
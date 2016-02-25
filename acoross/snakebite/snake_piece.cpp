#include "snake_piece.h"
#include "moving_object_system/moving_object_system.h"

namespace acoross {
namespace snakebite {

void SnakePiece::Move(const DirVector2D & diff_vec)
{
	Position2D pos_now = GetMovingObject().GetPosition();
	GetMovingObject().Move(diff_vec);

	if (snake_body_next_.get() != nullptr)
	{
		Position2D pos_body_next = snake_body_next_->GetMovingObject().GetPosition();
		DirVector2D diff_body_next{
			pos_now.x - pos_body_next.x,
			pos_now.y - pos_body_next.y,
		};

		double limitdist = (GetMovingObject().GetRadius() + snake_body_next_->GetMovingObject().GetRadius()) * 0.9;

		double piece_dist = diff_body_next.Length();
		if (piece_dist >= limitdist)
		{
			double diff_len = diff_vec.Length();

			diff_body_next = diff_body_next.GetNormalized();
			diff_body_next.x *= diff_len;
			diff_body_next.y *= diff_len;

			snake_body_next_->Move(diff_body_next);
		}
	}
}

}
}
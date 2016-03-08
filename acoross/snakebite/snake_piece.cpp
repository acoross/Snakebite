#include "snake_piece.h"
#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

//virtual override
void SnakePiece::Move(const DirVector2D & diff_vec)
{
	Position2D pos_now = GetPosition();
	MovingObject::Move(diff_vec);

	if (auto next = snake_body_next_.lock())
	{
		Position2D pos_body_next = next->GetPosition();
		DirVector2D diff_body_next{
			pos_now.x - pos_body_next.x,
			pos_now.y - pos_body_next.y,
		};

		double limitdist = (GetRadius() + next->GetRadius()) * 0.9;
		double piece_dist = diff_body_next.Length();

		if (piece_dist >= limitdist)
		{
			double diff_len = diff_vec.Length();

			diff_body_next = diff_body_next.GetNormalized();
			diff_body_next.x *= diff_len;
			diff_body_next.y *= diff_len;

			next->Move(diff_body_next);
		}
	}
}

}
}
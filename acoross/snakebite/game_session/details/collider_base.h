#ifndef ACOROSS_SNAKEBITE_COLLIDER_BASE_H_
#define ACOROSS_SNAKEBITE_COLLIDER_BASE_H_

namespace acoross {
namespace snakebite {

class SnakeCollider;
class AppleCollider;
class DummyCollider;

class SbColliderBase
{
public:
	virtual ~SbColliderBase() {}

	virtual void Collide(SbColliderBase& other, int cnt) = 0;
	virtual void Collide(SnakeCollider& other, int cnt) {}
	virtual void Collide(AppleCollider& other, int cnt) {}
	virtual void Collide(DummyCollider& other, int cnt) {}
};

#define ColliderImpl(T, OwnerT) \
class T : public SbColliderBase \
{	\
public:	\
	T(OwnerT* owner) : owner_(owner){}	\
	virtual void Collide(SbColliderBase& other, int cnt) override	\
	{	\
		other.Collide(*this, cnt + 1);	\
	}	\
	virtual void Collide(SnakeCollider& other, int cnt) override;	\
	virtual void Collide(AppleCollider& other, int cnt) override;	\
\
	OwnerT* owner_;	\
};


}
}
#endif //ACOROSS_SNAKEBITE_COLLIDER_BASE_H_

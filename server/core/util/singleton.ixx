export module util.singleton;

import <concepts>;
import <type_traits>;

export template<typename T>
class Singleton 
{
public:
    static T& Get() requires std::default_initializable<T>&& std::destructible<T> //memo : 생성 파괴 가능한 클래스 concepts
    {
        static T instance;
        return instance;
    }

protected:
    [[nodiscard]] Singleton() = default;
    virtual ~Singleton() = default;

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

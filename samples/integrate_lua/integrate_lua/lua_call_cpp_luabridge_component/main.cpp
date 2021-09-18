#include "game_object.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

lua_State* lua_state;

class Camera:public Component
{
public:
    void Awake() override{
        std::cout<<"Camera Awake"<<std::endl;
    }

    void Update() override{
        std::cout<<"Camera Update"<<std::endl;
    }

    void set_position(glm::vec3 position){
        std::cout<<"Camera set_position:"<<glm::to_string(position)<<std::endl;
        position_=position;
    }

    glm::vec3 position(){
        return position_;
    }

private:
    glm::vec3 position_;
};

class Animator:public Component
{
public:
    void Awake() override{
        std::cout<<"Animator Awake"<<std::endl;
    }

    void Update() override{
        std::cout<<"Animator Update"<<std::endl;
    }
};
//注册反射
RTTR_REGISTRATION
{
    registration::class_<Camera>("Camera")
            .constructor<>()(rttr::policy::ctor::as_raw_ptr);
    registration::class_<Animator>("Animator")
            .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

const int const_value=12;

typedef enum KeyAction{
    UP=0,
    DOWN=1,
    REPEAT=2
}KeyAction;

KeyAction GetKeyActionUp(){
    return KeyAction::UP;
}

KeyAction GetKeyActionDown(){
    return KeyAction::DOWN;
}

// 注册枚举
template <typename T>
struct EnumWrapper {
    static typename std::enable_if<std::is_enum<T>::value, void>::type push(lua_State* L, T value){
        lua_pushnumber (L, static_cast<std::size_t> (value));
    }
    static typename std::enable_if<std::is_enum<T>::value, T>::type get(lua_State* L, int index){
        return static_cast <T> (lua_tointeger (L, index));
    }
};

namespace luabridge {
    template <>
    struct Stack<KeyAction> : EnumWrapper<KeyAction>{};
} // namespace luabridge

int main(int argc, char * argv[])
{
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    luabridge::getGlobalNamespace(lua_state)
            .beginNamespace("Test")
            .addConstant("const_value",const_value)
            .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
            .beginNamespace("KeyAction")
            .addConstant<std::size_t>("UP",KeyAction::UP)
            .addConstant<std::size_t>("DOWN",KeyAction::DOWN)
            .endNamespace();

    luabridge::getGlobalNamespace(lua_state)
            .addFunction("GetKeyActionUp",&GetKeyActionUp)
            .addFunction("GetKeyActionDown",&GetKeyActionDown);

    luabridge::getGlobalNamespace(lua_state)
            .beginNamespace("glm")
            .beginClass<glm::vec3>("vec3")
            .addConstructor<void(*)(const float&, const float&, const float&)>()
            .addData("x", &glm::vec3::x)
            .addData("y", &glm::vec3::y)
            .addData("z", &glm::vec3::z)
            .addData("r", &glm::vec3::r)
            .addData("g", &glm::vec3::g)
            .addData("b", &glm::vec3::b)
            .addFunction ("__tostring", std::function <std::string (const glm::vec3*)> ([] (const glm::vec3* vec) {return glm::to_string(*vec);}))
            .endClass();
    luabridge::getGlobalNamespace(lua_state)
            .beginClass<GameObject>("GameObject")
            .addConstructor<void (*) ()>()
            .addFunction("__eq", &GameObject::operator==)
            .addFunction("AddComponent", (luabridge::LuaRef (GameObject::*)(std::string))&GameObject::AddComponentFromLua)
            .addFunction("GetComponent",&GameObject::GetComponentFromLua)
            .addFunction("test_set",&GameObject::test_set)
            .endClass();
    luabridge::getGlobalNamespace(lua_state)
            .beginClass<Component>("Component")
            .addConstructor<void (*) ()>()
            .addFunction("Awake",&Component::Awake)
            .addFunction("Update",&Component::Update)
            .addFunction("game_object",&Component::game_object)
            .addFunction("set_game_object",&Component::set_game_object)
            .endClass();
    luabridge::getGlobalNamespace(lua_state)
            .deriveClass<Animator,Component>("Animator")
            .addConstructor<void (*) ()>()
            .endClass();
    luabridge::getGlobalNamespace(lua_state)
            .deriveClass<Camera,Component>("Camera")
            .addConstructor<void (*) ()>()
            .addFunction("position",&Camera::position)
            .addFunction("set_position",&Camera::set_position)
            .endClass();

    //设置lua搜索目录
    {
        luabridge::LuaRef package_ref = luabridge::getGlobal(lua_state,"package");
        luabridge::LuaRef path_ref=package_ref["path"];
        std::string path=path_ref.tostring();
        path.append(";..\\?.lua;");
        package_ref["path"]=path;
    }

    luaL_dofile(lua_state, "../a.lua");

    //加上大括号，为了LuaRef在lua_close之前自动析构。
    {
        luabridge::LuaRef main_function = luabridge::getGlobal(lua_state, "main");
        try {
            main_function();
        } catch (const luabridge::LuaException& e) {
            std::cout<<"lua error: "<<e.what()<<std::endl;
        }
    }

    GameObject::Foreach([](GameObject* game_object){
        game_object->ForeachLuaComponent([](LuaRef lua_ref){
            LuaRef update_function_ref=lua_ref["Update"];
            if(update_function_ref.isFunction()){
                update_function_ref(lua_ref);
            }
        });
    });

    lua_close(lua_state);

    return 0;
}

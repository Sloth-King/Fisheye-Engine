# Fisheye Engine
<img width="814" height="725" alt="fisheye logo" src="https://github.com/user-attachments/assets/5263b36a-cd05-40bf-9dc1-ac26ad6d597f" />

Fisheye is a 3D game engine written and usable in C++. The intention was to create an easy to use, performant, and customizable platform for game developement.

> This is a university project made by two people in ~two months.
> 
> No generative AI was used in the making of this project.

## Features



> The physics engine is still experimental. Things may not work as expected.

## How To Use

We'll create a very simple game to show how one may use the engine.

### Initial setup
Let's start by creating a *Game* in our main function, setting some of its parameters, initialize openGL and start the update loops.

```cpp
#include "engine/includes/core.h" // imports core engine features
#include "engine/includes/components.h" // imports base engine components
void main (){
  Game game;

  game.settings.windowWidth = 1280;
  game.settings.windowHeight = 720;
  
  game.init(); // initialize graphical backend

  setupGameScene(game); // function detailed in the next part

  game.start(); // start update and physics loops
  game.waitGameStopped(); // wait for the quit signal before terminating

}
```

Build and launch using the provided Cmake. This gives us a fully black window.


### Cube and light

Let us add a cube and a light to our scene. We fill up the *setupScene* function called in our main.

```cpp
void setupGameScene(){
  
oid setupScene(Game& game){
    Scene scene; // Create a scene to contain our gameobjects
    GameObject world; // root of our scene

    // load an obj file
    Mesh cube = ResourceLoader::load_mesh_obj("../game/resources/meshes/supercube.obj");

    // We create a PBR material object. A handle is the (possibly shared) owner of a resource
    // As long as one handle exists, the resource is not freed up (akin to a shared_ptr)
    cube.material = Handle<MaterialPBR>(Texture("../game/resources/textures/logo.jpg"));

    // Add a 'Transform' compoment to the world
    // addComponent creates the component on the gameobject and returns a pointer to it.
    // The pointer is guaranteed to be valid for the lifetime of the gameobject.
    C_Transform* t = world->addComponent<C_Transform>(); 
    t->setPosition(glm::vec3(0, -0.2, 0)); // using glm as a placeholder, as we'll be creating our own algebra module.
    t->setScale(glm::vec3(0.1, 0.1, 0.1));

    // add a mesh component (directly on the world for now)
    world->addComponent<C_Mesh>() -> mesh = cube; 

    // light
    GameObject light;
    auto* lightComponent = light->addComponent<C_Light>();
    lightComponent->light.color = glm::vec3(1.0, 1.0, 1.0);
    light->addComponent<C_Transform>()->setPosition(glm::vec3(10.0, 10.0, 5.0));

    world->addChild(std::move(light)); // We give the light to the world (amen)
    lightComponent->light.intensity = 100.0; // but the discarded object is still valid to use as a non-owning pointer to the gameobject

    scene.setRoot(std::move(world)); // we give the world to the scene
    game.setScene(std::move(scene)); // and the scene to the game
}
```

We get the following scene:

<img width="1253" height="752" alt="one cube in the void" src="https://github.com/user-attachments/assets/b7573937-311c-4c0c-a028-f8245cca8fe3" />


From there, we could add more components to further customize the behavior of our gameobjects.

### Implementing new behaviors

In order to customize the behavior of the gameobjects in our world, we can create new components. We do this by inheriting from the *Component* class.

This gives us five virtual functions that we may override to our liking :

- *onEnterScene() & onExitScene()*, called when the owning gameobject enters or exits a scene
- *onUpdate($\Delta_t$)), onPhysicsUpdate($\Delta_t$) et onLateUpdate($\Delta_t$)* are called at different stages of the frame's computation.

> For now, the update functions are called synchronously. In the future, *update* and *physicsUpdate* will each have their own thread.

By inheriting other components (and creating new virtual functions to override), we can easily and organically expand the capabilities of our gameobjects. 

For example, the *C_RigidBody* Component offers an *onContact(Contact)* function, which allows inheriting components to execute code on a physics collision.

Les composants peuvent évidemment hériter d'autres composants, et créer de nouvelles fonctions virtuelles pour offrir des options aux composants héritants, par exemple, la classe de composant *C_RigidBody* offre une fonction *onContact(contact c)* qui permet à un composant en héritant d'exécuter du code sur une collision physique.

 Let's create a player controller component. This also demonstrates the user inputs system:

```cpp
class C_PlayerController: public Component {
public:
    float movement_speed = 2.0;
    void jump();

    void inputCallback(const InputEvent & e){
        if (e == "move_up" && e.pressed() && isOnGround){
            jump(); 
    }
    C_PlayerController(){
      // Exemple d'utilisation du gestionnaires d'entrées utilisateur.
      // The lambda is necessary as we can't cast a method to a generic function signature.
      Input::addInputListener(
          [& /*capture ambient scope*/](const InputEvent & e){
              inputCallback(e);
          }
      );

      // définies ici pour l'exemple mais c'est mieux de le faire clairement au lancement du jeu
      Input::addKeybind( "move_right",
        KeySpec(BINDTYPE_KEYBOARD, GLFW_KEY_D)
      );
      Input::addKeybind("move_left",
        KeySpec(BINDTYPE_KEYBOARD, GLFW_KEY_A) 
      );
      Input::addKeybind("move_up",
        KeySpec(BINDTYPE_MOUSE, GLFW_MOUSE_BUTTON_1) 
      );
  }

  virtual void _onUpdate(float delta) override{

      delta *= movement_speed;

      // si notre gameobject n'a pas de transform, on sort.
      if (!getOwner()->hasComponent<C_Transform>()) return;

      auto* transform = getOwner()->getComponent<C_Transform>();

      if (Input::isInputPressed("move_right")){
          transform->move(glm::vec3(1, 0, 0) * delta);
      }
      if (Input::isInputPressed("move_left")){
          transform->move(glm::vec3(-1, 0, 0) * delta);
      }
  }
};

```

Il suffit ensuite d'ajouter ce composant sur un gameobject pour obtenir les comportements qu'il décrit.

#boxed[Pour l'instant, seul 9 composants sont disponibles dans le moteur :
- C_Transform
- C_RigidBody
- C_Collider
- C_Mesh
- C_Camera
- C_Light
- C_PlayerController\*
- C_MapManager\*
- C_voxelMesh\*

_\*créés pour la démo technique du sous marin_

Nous aimerions à terme avoir une librairie de composants divers et variés afin de faciliter le développement de jeux.
]



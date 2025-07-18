# Fisheye Engine
<img width="814" height="725" alt="fisheye logo" src="https://github.com/user-attachments/assets/5263b36a-cd05-40bf-9dc1-ac26ad6d597f" />

Fisheye is a 3D game engine written and usable in C++. The intention was to create an easy to use, performant, and customizable platform for game developement.

> This is a university project made by two people in ~two months.
> 
> No generative AI was used in the making of this project.

## Features

## How To Use

We'll create a very simple game to show how one may use the engine.

### Initial setup
Let's start by creating a *Game*

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

Let us add a cube and a light to our scene.

```cpp
void setupGameScene(){
  
oid setupScene(Game& game){
    Scene scene; // On crée une scène qui contiendra nos objet
    GameObject world; // On crée notre objet racine

    // chargement d'un obj
    Mesh cube = ResourceLoader::load_mesh_obj("../game/resources/meshes/supercube.obj");

    // On crée notre material. Les handles sont des objets référence à des ressources.
    //Tant qu'au moins une handle existe, la ressource n'est pas libérée.
    cube.material = Handle<MaterialPBR>(Texture("../game/resources/textures/logo.jpg"));

    // On ajoute un composant Transform à notre monde
    // ainsi qu'un composant mesh (pour l'instant directement sur le monde)
    // addComponent renvoie un pointeur vers le composant nouvellement créé, afin de le modifier
    C_Transform* t = world->addComponent<C_Transform>(); 
    t->setPosition(glm::vec3(0, -0.2, 0));
    t->setScale(glm::vec3(0.1, 0.1, 0.1));

    world->addComponent<C_Mesh>() -> mesh = cube; 

    // light

    GameObject light;

    auto* lightComponent = light->addComponent<C_Light>();
    lightComponent->light.color = glm::vec3(1.0, 1.0, 1.0);
    light->addComponent<C_Transform>()->setPosition(glm::vec3(10.0, 10.0, 5.0));

    world->addChild(std::move(light)); // On donne l'ownership de la lumière au world
    lightComponent->light.intensity = 100.0; // mais on peut encore la modifier de manière valide.

    scene.setRoot(std::move(world));
    game.setScene(std::move(scene)); // on donne l'ownership de la scène au jeu
}
```

On obtient la scène suivante :

<img width="1253" height="752" alt="image" src="https://github.com/user-attachments/assets/b7573937-311c-4c0c-a028-f8245cca8fe3" />


A partir de là, on pourrait par exemple ajouter de la physique, une skybox avec des cubemaps, des interactions utilisateur  l'aide de la classe Input (voir l'exemple dans la partie suivante).

### Implementing and assigning behaviors

Afin de customiser les comportements des objets, on peut créer de nouveaux composants, par héritage de la classe *Component*. Cette classe possède cinq fonctions virtuelles que l'utilisateur peut override pour exécuter la logique qu'il souhaite :
- *onEnterScene() et onExitScene()*, sont appelées respectivement quand le composant entre et sort de la scène (ou est détruit)
- *onUpdate($Delta_t$)), onPhysicsUpdate($Delta_t$) et onLateUpdate($Delta_t$)* sont appelées à différentes étapes de la boucle de mise à jour. Pour l'instant, elles le sont de manière synchrone. Dans le futur, la physique et le rendu seront sur deux threads séparés donc l'ordre d'exécution et les $Delta_t$ ne seront pas forcément les mêmes.

Les composants peuvent évidemment hériter d'autres composants, et créer de nouvelles fonctions virtuelles pour offrir des options aux composants héritants, par exemple, la classe de composant *C_RigidBody* offre une fonction *onContact(contact c)* qui permet à un composant en héritant d'exécuter du code sur une collision physique.

Voici un exemple de composant utilisateur :

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
      // Une lambda est nécessaire car les méthodes sont pas génériquement pointables pour c++
      Input::addInputListener(
          [& /*capture l'objet ambient*/](const InputEvent & e){
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



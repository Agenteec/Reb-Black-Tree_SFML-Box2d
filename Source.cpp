#include <iostream>
#include <SFML/Graphics.hpp>
#include <iomanip>
#include <box2d/box2d.h>
#include <box2d/b2_settings.h>
#include <box2d/b2_world.h>


#include <cstdlib>
#include <ctime>

sf::Font font;
#define Grav -40.f

float PTM_RATIO = 1;
enum class Color {
    RED,
    BLACK
};

template <typename T>
struct Node {
    T value;
    Color color;
    Node* left;
    Node* right;
    Node* parent;
    b2Body* body;
    Node(T value) : value(value), color(Color::RED), left(nullptr), right(nullptr), parent(nullptr),body(nullptr) {
    
    }
};

template <typename T>
class RedBlackTree {
     size_t nodes_count;
     
public:
    b2World* scene;
    RedBlackTree() :
        root(nullptr),
        nodes_count(0){
    
        // Create the Box2D world
        b2Vec2 gravity(0.f, Grav);
        bool doSleep = true;
        scene = new b2World(gravity);
        scene->SetAllowSleeping(doSleep);

    }
    ~RedBlackTree() { 
        destroyTree(root); 
        delete scene;
    }

    void insert(T value);
    void remove(T value);
    bool contains(T value);
    void print(sf::RenderWindow* window = nullptr);
    void traverseInorder(void (*func)(T value));

private:
    Node<T>* root;
    void Box2dInit(Node<T>* node, int x, int y, int h);
    void destroyTree(Node<T>* node);
    void insertFixup(Node<T>* node);
    void removeFixup(Node<T>* node);
    Node<T>* findNode(T value);
    Node<T>* findMinimum(Node<T>* node);
    void transplant(Node<T>* u, Node<T>* v);
    void printTree(Node<T>* node, sf::RenderWindow* window, int x, int y, int h);
    void rotateLeft(Node<T>* node);
    void rotateRight(Node<T>* node);
};

template <typename T>
void RedBlackTree<T>::insert(T value) {
    //Всё как в обычной бинарке
    //Новый листок
    Node<T>* node = new Node<T>(value);

    Node<T>* parent = nullptr;
    Node<T>* current = root;
    while (current != nullptr) {
        parent = current;
        if (value < current->value) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }

    node->parent = parent;
    //Если node сирота
    if (parent == nullptr) {
        root = node;
    }
    else if (value < parent->value) {
        parent->left = node;
    }
    else {
        parent->right = node;
    }

    insertFixup(node);
}

template <typename T>
void RedBlackTree<T>::insertFixup(Node<T>* node) {
    while (node->parent != nullptr && node->parent->color == Color::RED) {
        if (node->parent == node->parent->parent->left) {
            Node<T>* uncle = node->parent->parent->right;
            if (uncle != nullptr && uncle->color == Color::RED) {
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            }
            else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rotateLeft(node);
                }
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateRight(node->parent->parent);
            }
        }
        else {
            Node<T>* uncle = node->parent->parent->left;
            if (uncle != nullptr && uncle->color == Color::RED) {
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            }
            else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rotateRight(node);
                }
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateLeft(node->parent->parent);
            }
        }
    }
    root->color = Color::BLACK;
}

template <typename T>
void RedBlackTree<T>::removeFixup(Node<T>* node) {
    while (node != root && node->color == Color::BLACK) {
        if (node == node->parent->left) {
            Node<T>* sibling = node->parent->right;
            if (sibling->color == Color::RED) {
                sibling->color = Color::BLACK;
                node->parent->color = Color::RED;
                rotateLeft(node->parent);
                sibling = node->parent->right;
            }
            if (sibling->left->color == Color::BLACK && sibling->right->color == Color::BLACK) {
                sibling->color = Color::RED;
                node = node->parent;
            }
            else {
                if (sibling->right->color == Color::BLACK) {
                    sibling->left->color = Color::BLACK;
                    sibling->color = Color::RED;
                    rotateRight(sibling);
                    sibling = node->parent->right;
                }
                sibling->color = node->parent->color;
                node->parent->color = Color::BLACK;
                sibling->right->color = Color::BLACK;
                rotateLeft(node->parent);
                node = root;
            }
        }
        else {
            Node<T>* sibling = node->parent->left;
            if (sibling->color == Color::RED) {
                sibling->color = Color::BLACK;
                node->parent->color = Color::RED;
                rotateRight(node->parent);
                sibling = node->parent->left;
            }
            if (sibling->right->color == Color::BLACK && sibling->left->color == Color::BLACK) {
                sibling->color = Color::RED;
                node = node->parent;
            }
            else {
                if (sibling->left->color == Color::BLACK) {
                    sibling->right->color = Color::BLACK;
                    sibling->color = Color::RED;
                    rotateLeft(sibling);
                    sibling = node->parent->left;
                }
                sibling->color = node->parent->color;
                node->parent->color = Color::BLACK;
                sibling->left->color = Color::BLACK;
                rotateRight(node->parent);
                node = root;
            }
        }
    }
    node->color = Color::BLACK;
}

template <typename T>
void RedBlackTree<T>::rotateLeft(Node<T>* node) {
    Node<T>* right_child = node->right;
    node->right = right_child->left;
    if (right_child->left != nullptr) {
        right_child->left->parent = node;
    }
    right_child->parent = node->parent;
    if (node->parent == nullptr) {
        root = right_child;
    }
    else if (node == node->parent->left) {
        node->parent->left = right_child;
    }
    else {
        node->parent->right = right_child;
    }
    right_child->left = node;
    node->parent = right_child;
}

template <typename T>
void RedBlackTree<T>::rotateRight(Node<T>* node) {
    Node<T>* left_child = node->left;
    node->left = left_child->right;
    if (left_child->right != nullptr) {
        left_child->right->parent = node;
    }
    left_child->parent = node->parent;
    if (node->parent == nullptr) {
        root = left_child;
    }
    else if (node == node->parent->right) {
        node->parent->right = left_child;
    }
    else {
        node->parent->left = left_child;
    }
    left_child->right = node;
    node->parent = left_child;
}

template <typename T>
void RedBlackTree<T>::transplant(Node<T>* u, Node<T>* v) {
    if (u->parent == nullptr) {
        root = v;
    }
    else if (u == u->parent->left) {
        u->parent->left = v;
    }
    else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

template <typename T>
Node<T>* RedBlackTree<T>::findNode(T value) {
    Node<T>* current = root;
    while (current != nullptr) {
        if (value == current->value) {
            return current;
        }
        else if (value < current->value) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }
    return nullptr;
}

template <typename T>
Node<T>* RedBlackTree<T>::findMinimum(Node<T>* node) {
    while (node->left != nullptr) {
        node = node->left;
    }
    return node;
}

template<typename T>
void RedBlackTree<T>::destroyTree(Node<T>* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
    nodes_count = 0;
    root = nullptr;
}

template<typename T>
void RedBlackTree<T>::traverseInorder(void (*func)(T value)) {
    Node<T>* current = root;
    Node<T>* pre;

    while (current != nullptr) {
        if (current->left == nullptr) {
            (*func)(current->value);
            current = current->right;
        }
        else {
            pre = current->left;
            while (pre->right != nullptr && pre->right != current)
                pre = pre->right;

            if (pre->right == nullptr) {
                pre->right = current;
                current = current->left;
            }
            else {
                pre->right = nullptr;
                (*func)(current->value);
                current = current->right;
            }
        }
    }
}

template <typename T>
void RedBlackTree<T>::remove(T value) {
    Node<T>* node = findNode(value);
    if (node == nullptr) {
        return; // элемент не найден
    }

    Node<T>* x = nullptr;
    Node<T>* y = node;
    Color y_original_color = y->color;

    if (node->left == nullptr) {
        x = node->right;
        transplant(node, node->right);
    }
    else if (node->right == nullptr) {
        x = node->left;
        transplant(node, node->left);
    }
    else {
        y = findMinimum(node->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == node) {
            x->parent = y;
        }
        else {
            transplant(y, y->right);
            y->right = node->right;
            y->right->parent = y;
        }
        transplant(node, y);
        y->left = node->left;
        y->left->parent = y;
        y->color = node->color;
    }

    if (y_original_color == Color::BLACK) {
        removeFixup(x);
    }

    nodes_count--;
}

template <typename T>
void RedBlackTree<T>::print(sf::RenderWindow* window)
{
    if (window != nullptr)
    {
        printTree(root, window, 400, 50, 50);
    }
    else
    {
        Box2dInit(root, 400, 50, 50);
    }
    
}

//template <typename T>
//void RedBlackTree<T>::printTree(Node<T>* node, sf::RenderWindow* window, int x, int y, int h)
//{
//    if (node == nullptr)
//    {
//        return;
//    }
//
//    sf::CircleShape circle(20.f);
//    circle.setOrigin(circle.getRadius(), circle.getRadius());
//    circle.setPosition(x, y);
//
//    if (node->color == Color::BLACK)
//    {
//        circle.setFillColor(sf::Color::Black);
//    }
//    else
//    {
//        circle.setFillColor(sf::Color::Red);
//    }
//
//   
//
//    sf::Font font;
//    font.loadFromFile("arial.ttf");
//    sf::Text text(std::to_string(node->value), font, 20);
//    text.setFillColor(sf::Color::White);
//    text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
//    text.setPosition(x, y);
//
//   
//
//    if (node->left != nullptr)
//    {
//        sf::Vertex line[] = { sf::Vertex(sf::Vector2f(x, y)), sf::Vertex(sf::Vector2f(x - h, y + h)) };
//        window->draw(line, 2, sf::Lines);
//
//        printTree(node->left, window, x - h, y + h, h);
//    }
//
//    if (node->right != nullptr)
//    {
//        sf::Vertex line[] = { sf::Vertex(sf::Vector2f(x, y)), sf::Vertex(sf::Vector2f(x + h, y + h)) };
//        window->draw(line, 2, sf::Lines);
//        printTree(node->right, window, x + h, y + h, h);
//    }
//    window->draw(circle);
//    window->draw(text);
//}
float dyt = 1.5f;
template <typename T>
void RedBlackTree<T>::printTree(Node<T>* node, sf::RenderWindow* window, int x, int y, int h)
{
    if (node == nullptr) {
        return;
    }
    const b2Vec2& position = node->body->GetPosition();
    // Draw a line to the left child
    if (node->left != nullptr)
    {
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(position.x, position.y + window->getSize().y/ dyt), sf::Color::White),
            sf::Vertex(sf::Vector2f(node->left->body->GetPosition().x, node->left->body->GetPosition().y + window->getSize().y/ dyt), sf::Color::White)
        };
        window->draw(line, 2, sf::Lines);
    }

    // Draw a line to the right child
    if (node->right != nullptr)
    {
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(position.x, position.y + window->getSize().y/ dyt), sf::Color::White),
            sf::Vertex(sf::Vector2f(node->right->body->GetPosition().x, node->right->body->GetPosition().y + window->getSize().y/ dyt), sf::Color::White)
        };
        window->draw(line, 2, sf::Lines);
    }

    // Draw the left subtree
    printTree(node->left, window, x - h, y + h, h / 2);

    // Draw the node
    sf::CircleShape circle(20.f);
    circle.setOrigin(circle.getRadius(), circle.getRadius());

    

    node->body->ApplyLinearImpulse(b2Vec2(1, 1), node->body->GetPosition(), true);
    circle.setPosition(position.x, position.y + window->getSize().y/ dyt);

    if (node->color == Color::BLACK)
    {
        circle.setFillColor(sf::Color::Black);
    }
    else
    {
        circle.setFillColor(sf::Color::Red);
    }

    window->draw(circle);
    //std::cout << circle.getPosition().x << "\t" << circle.getPosition().y << "\t" <<node->value<< "\t" << (node->color==Color::RED?"RED":"BLACK") << std::endl;
    //system("pause");

    // Draw the value
    sf::Text text(std::to_string(node->value), font, 20);
    text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
    text.setPosition(position.x, position.y + window->getSize().y/ dyt);
    window->draw(text);

    

    // Draw the right subtree
    printTree(node->right, window, x + h, y + h, h / 2);
}
//void RedBlackTree<T>::printTree(Node<T>* node, sf::RenderWindow* window, int x, int y, int h)
//{
//    if (node == nullptr) {
//        return;
//    }
//    
//     Draw the left subtree
//    printTree(node->left, window, x - h, y + h, h / 2);
//
//     Draw the node
//    sf::CircleShape circle(20.f);
//    circle.setOrigin(circle.getRadius(), circle.getRadius());
//
//    const b2Vec2& position = node->body->GetPosition();
//    node->body->ApplyLinearImpulse(b2Vec2(1,1), node->body->GetPosition(), true);
//    circle.setPosition(position.x, position.y+500);
//
//    if (node->color == Color::BLACK)
//    {
//        circle.setFillColor(sf::Color::Black);
//    }
//    else
//    {
//        circle.setFillColor(sf::Color::Red);
//    }
//
//    window->draw(circle);
//    std::cout << circle.getPosition().x<<"  " << circle.getPosition().y << std::endl;
//    system("pause");
//
//     Draw the value
//    sf::Text text(std::to_string(node->value), font, 20);
//    text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
//    text.setPosition(position.x, position.y+500);
//    window->draw(text);
//
//     Draw the right subtree
//    printTree(node->right, window, x + h, y + h, h / 2);
//}
template <typename T>
void RedBlackTree<T>::Box2dInit(Node<T>* node, int x, int y, int h)
{
    if (node == nullptr)
    {
        return;
    }

    // Create a Box2D body for the node
    b2BodyDef bodyDef;
    if (node->parent == nullptr) {
        bodyDef.type = b2_staticBody;  // корень фиксированный
        bodyDef.fixedRotation = true; // корень не вращается
    }
    else {
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = false;
    }
    bodyDef.position.Set(x, y);
    node->body = scene->CreateBody(&bodyDef); // сохраняем указатель на созданное тело

    // Create a Box2D circle shape for the node
    b2CircleShape circleShape;
    circleShape.m_radius = 20.f / PTM_RATIO;
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = (node->parent == nullptr) ? 0.0f : 0.5f; // корень без плотности, остальные с отрицательной плотностью
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = (node->parent == nullptr) ? 1.0f : 0.8f; // корень с максимальной упругостью, остальные с более низкой
    node->body->CreateFixture(&fixtureDef);


    // Create a joint between the node and its parent
    if (node->parent != nullptr)
    {
        b2DistanceJointDef jointDef;
        jointDef.bodyA = node->body;
        jointDef.bodyB = node->parent->body;
        jointDef.localAnchorA.Set(0.f, 0.f);
        jointDef.localAnchorB.Set(0.f, -h / PTM_RATIO);
        jointDef.length = (node->body->GetPosition() - node->parent->body->GetPosition()+b2Vec2(500, 500)).Length();
        jointDef.Initialize(node->body, node->parent->body, node->body->GetWorldCenter(), node->parent->body->GetWorldCenter());
        scene->CreateJoint(&jointDef);
    }

    int rootX = h;
    // Calculate the initial x-coordinate of the root node
    if (node->parent != nullptr)
    {
        //rootX = h * (pow(2, node->parent->body->GetPosition().x - 1) / 2);
    }
    // Recursively create objects for the children
    if (node->left != nullptr)
    {
        Box2dInit(node->left, x - 1 / 2 - h, y - h, rootX);

        // Create a joint between the node and its left child
        b2DistanceJointDef jointDef;
        jointDef.bodyA = node->body;
        jointDef.bodyB = node->left->body;
        jointDef.localAnchorA.Set(0.f, 0.f);
        jointDef.localAnchorB.Set(0.f, -h / PTM_RATIO);
        jointDef.length = (node->body->GetPosition() - node->left->body->GetPosition() + b2Vec2(500, 500)).Length() ;
        jointDef.Initialize(node->body, node->left->body, node->body->GetWorldCenter(), node->left->body->GetWorldCenter());
        scene->CreateJoint(&jointDef);
    }
    if (node->right != nullptr)
    {
        Box2dInit(node->right, x + 1 / 2 + h, y - h, rootX);
        // Create a joint between the node and its right child
        b2DistanceJointDef jointDef;
        jointDef.bodyA = node->body;
        jointDef.bodyB = node->right->body;
        jointDef.localAnchorA.Set(0.f, 0.f);
        jointDef.localAnchorB.Set(0.f, -h / PTM_RATIO);
        jointDef.length = (node->body->GetPosition() - node->right->body->GetPosition() + b2Vec2(500, 500)).Length() ;
        jointDef.Initialize(node->body, node->right->body, node->body->GetWorldCenter(), node->right->body->GetWorldCenter());
        scene->CreateJoint(&jointDef);
    }
    //std::cout << node->body->GetPosition().x << "\t" << node->body->GetPosition().y << "\t" << node->value << "\t" << (node->color == Color::RED ? "RED" : "BLACK") << std::endl;
    //system("pause");
}
//void RedBlackTree<T>::Box2dInit(Node<T>* node, int x, int y, int h)
//{
//    if (node == nullptr)
//    {
//        return;
//    }
//
//    // Create a Box2D body for the node
//    b2BodyDef bodyDef;
//    bodyDef.type = b2_dynamicBody;
//    bodyDef.position.Set(x, y);
//    node->body = scene->CreateBody(&bodyDef); // сохраняем указатель на созданное тело
//
//    // Create a Box2D circle shape for the node
//    b2CircleShape circleShape;
//    circleShape.m_radius = 20.f / PTM_RATIO;
//    b2FixtureDef fixtureDef;
//    fixtureDef.shape = &circleShape;
//    fixtureDef.density = 1.f;
//    fixtureDef.friction = 0.3f;
//    fixtureDef.restitution = 0.5f;
//    node->body->CreateFixture(&fixtureDef);
//
//    // Create a joint between the node and its parent
//    if (node->parent != nullptr)
//    {
//        b2RevoluteJointDef jointDef;
//        jointDef.bodyA = node->body;
//        jointDef.bodyB = node->parent->body;
//        jointDef.localAnchorA.Set(0.f, 0.f);
//        jointDef.localAnchorB.Set(0.f, -h / PTM_RATIO);
//        scene->CreateJoint(&jointDef);
//    }
//
//    // Recursively create objects for the children
//    if (node->left != nullptr)
//    {
//        Box2dInit(node->left, x - h, y + h, h);
//
//        // Create a joint between the node and its left child
//        b2RevoluteJointDef jointDef;
//        jointDef.bodyA = node->body;
//        jointDef.bodyB = node->left->body;
//        jointDef.localAnchorA.Set(-h / PTM_RATIO, h / PTM_RATIO);
//        jointDef.localAnchorB.Set(0.f, -h / PTM_RATIO);
//        scene->CreateJoint(&jointDef);
//    }
//    if (node->right != nullptr)
//    {
//        Box2dInit(node->right, x + h, y + h, h);
//
//        // Create a joint between the node and its right child
//        b2RevoluteJointDef jointDef;
//        jointDef.bodyA = node->body;
//        jointDef.bodyB = node->right->body;
//        jointDef.localAnchorA.Set(h / PTM_RATIO, h / PTM_RATIO);
//        jointDef.localAnchorB.Set(0.f, -h / PTM_RATIO);
//        scene->CreateJoint(&jointDef);
//    }
//}

void f(int x)
{
    std::cout << x << std::endl;
}
void Render(sf::RenderWindow* window);
int main()
{
    font.loadFromFile("arial.ttf");
	sf::RenderWindow* window;
	window = new sf::RenderWindow(sf::VideoMode(1024,800), "Tree", sf::Style::Close);
    window->setFramerateLimit(60);
    Render(window);
}
void Render(sf::RenderWindow* window)
{

    srand(static_cast<unsigned int>(time(nullptr)));

    int min = 0;
    int max = 1000;

    RedBlackTree<int> Tree;
    for (int i = 0; i < 50;i++)
    {
        int randomNumber = rand() % (max - min + 1) + min;
        Tree.insert(randomNumber);
        

    }
    Tree.print();
    #pragma region Render
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {

            if (event.type == sf::Event::Closed)
                window->close();
        }
        window->clear(sf::Color(66, 135, 245));
        Tree.print(window);

        Tree.scene->Step(1.f / 60.0f, 8, 1);
        window->display();
    }
    #pragma endregion
}
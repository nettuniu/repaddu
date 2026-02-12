namespace sample {
class Base {
public:
    virtual void run() = 0;
};
class Derived : public Base {
public:
    void run() override {}
};
}

#pragma once

class Example
{
public:
    Example();
    Example(int a);

    void setValue(int v);
    int  getValue(void);

private:
    int value;
};


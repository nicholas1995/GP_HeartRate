
int get_a(void)
{
    int a =5;
    return a;
}

void set_a(int *ptr)
{
    *ptr=*ptr++;
}
int main()
{
    int b;
    b=get_a();
}
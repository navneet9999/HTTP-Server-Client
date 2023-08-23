#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp1, *fp2;
    fp1 = fopen("image.jpg", "rb");
    fp2 = fopen("copy.jpg", "wb");
    int a;
    while((a = fgetc(fp1)) != EOF)
    {
        fputc(a, fp2);
    }
    return 0;
}
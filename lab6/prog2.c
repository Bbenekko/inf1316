int main (void)
{
    int fifo;
    char ch;
    if ((fifo = open("minhaFifo", "r")) < 0)
    {
        puts ("Erro ao abrir a FIFO para escrita"); return -1;
    }
    while (read(fifo, &ch, sizeof(ch)) > 0) putchar (ch);
    close (fifo);
    return 0;
} 
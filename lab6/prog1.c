int main (void)
{
    int fifo;
    char c;
    if ((fifo = open("minhaFifo", "w")) < 0)
    {
        puts ("Erro ao abrir a FIFO para escrita"); return -1;
    }
    write(fifo, &c, sizeof(c));
    close (fifo);
    return 0;
}
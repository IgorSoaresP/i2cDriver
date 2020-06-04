------------------------------------------------------------------------
Alunos: 
    Igor dos Santos
    Luis Fernando Segalla
------------------------------------------------------------------------

Instruções para a correta compilação e execução do trabalho:

1 - Abra o terminal na pasta TRABALHO_FINAL_SO_LUIS_E_IGOR
2 - Digite o comando make para compilar o módulo
3 - Use o comando make carrega para carregar o módulo no kernel
4 - Use o comando cat /proc/devices e procure por DriverSOi2c, ao seu lado deve haver seu valor de major number. Confira se este valor bate com o presente na instrução no, se não for compatível modifique o arquivo makefile para o valor encontrado.
5 - Use o comando make no

------------------------------------------------------------------------

Rodando a aplicação:

1 - Use o comando make run, isso irá compilar e rodar a aplicação

------------------------------------------------------------------------

Removendo o módulo e a aplicação:

1 - Para a correta remoção do módulo use o comando make descarrega
2 - Para deletar a aplicação use o comando make deleteApp
3 - Para deletar todos os arquivos de compilação use o comando make limpa

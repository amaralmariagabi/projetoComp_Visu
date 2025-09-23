#Raphaela Polonis 10408843
#Maria Gabriela 10409037
#Livia 10389419
#Isabella 10357696
# Projeto Visual - Processamento de Imagens com SDL3

## 📌 Descrição
Este projeto implementa um sistema de processamento de imagens utilizando a **biblioteca SDL3** e suas extensões (`SDL_image` e `SDL_ttf`).  
A aplicação permite:
- Carregar uma imagem em tons de cinza.
- Calcular e exibir o histograma de intensidades.
- Aplicar **equalização de histograma** para melhorar o contraste.
- Exibir a classificação automática da imagem quanto à **luminosidade** (escura, média ou clara) e **contraste** (baixo, médio ou alto).
- Permitir salvar a versão atual da imagem em disco ao pressionar a tecla `S`.

## 🖥️ Funcionalidades
- **Conversão para escala de cinza**  
  A imagem original é convertida para tons de cinza usando a fórmula ponderada de luminosidade.

- **Cálculo do histograma, média e desvio padrão**  
  Usado para classificar a luminosidade e contraste da imagem.

- **Equalização de histograma**  
  Melhora a qualidade visual da imagem, ajustando os níveis de intensidade.

- **Interface gráfica com SDL3**  
  - Janela principal: exibe a imagem (original em cinza ou equalizada).  
  - Janela secundária: mostra o histograma, a classificação e um botão interativo para alternar entre versões.

- **Interatividade**  
  - Botão "Equalizar" / "Original" para alternar a visualização.  
  - Pressionar `S` salva a imagem atual como `output_image.png`.

---

Contribuições do Grupo

Maria Gabriela Barros

Implementação da conversão para escala de cinza.

Funções de histograma, cálculo de média e desvio padrão.

Escrita inicial do README.

Participação em testes e depuração.

Raphaela Polonis

Implementação da interface gráfica com SDL3.

Integração do botão de equalização e salvamento da imagem.

Estruturação do repositório e organização de commits.

Revisão final da documentação.

Livia

Apoio na pesquisa de bibliotecas (SDL3, SDL_image, SDL_ttf).

Auxílio na equalização de histograma.

Testes de execução e validação da aplicação.


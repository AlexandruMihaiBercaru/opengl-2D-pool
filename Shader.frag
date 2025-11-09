//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Cursul IV - 04_01_Shader.frag     |
// =====================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

uniform int codCol;

void main(void)
  {
    switch(codCol)
    {
        case 0: //NoChange
            out_Color = ex_Color;
            break;
        case 1://Red
            out_Color = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        case 2: //LightBlue
            out_Color = vec4(0.0, 1.0, 1.0, 1.0);
            break;
        case 3: //White
            out_Color = vec4(1.0, 1.0, 1.0, 1.0);
        default:
            out_Color = ex_Color;
            break;
    }
  }
 
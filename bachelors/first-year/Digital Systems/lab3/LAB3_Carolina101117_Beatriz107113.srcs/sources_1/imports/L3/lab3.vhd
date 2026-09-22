--------------------------------------------------------------
-- 7: <n�mero do grupo>
-- <BEATRIZ COELHO> (<107113>)
-- <CAROLINA BALTAZAR> (<107117>)
--------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
-- library UNISIM;
-- use UNISIM.VComponents.all;

entity Lab3 is
    Port ( A : in STD_LOGIC_VECTOR (4 downto 0);
           B : in STD_LOGIC_VECTOR (4 downto 0);
           I : in STD_LOGIC;
           S : out STD_LOGIC_VECTOR (7 downto 0));
end Lab3;

architecture Behavioral of lab3 is
-- declaracao do componente somador8
component somador8 
    Port ( P : in STD_LOGIC_VECTOR (7 downto 0);
           Q : in STD_LOGIC_VECTOR (7 downto 0);
           cin : in STD_LOGIC;
           R : out STD_LOGIC_VECTOR (7 downto 0);
          cout : out STD_LOGIC);
end component;

-- declaracao dos sinais internos
signal sign_A, sign_B : std_logic_vector (7 downto 0);
signal AL2, BL1, som1mod, som3mod: std_logic_vector (7 downto 0);
signal notA: std_logic_vector (7 downto 0);

signal som1_P, som1_Q, som1_S : std_logic_vector (7 downto 0);
signal som1_cin : std_logic;

signal som2_P, som2_Q, som2_S : std_logic_vector (7 downto 0);
signal som2_cin : std_logic;

signal som3_P, som3_Q, som3_S : std_logic_vector (7 downto 0);
signal som3_cin : std_logic;


-- declaracao de sinais adicionais que possam ser nencessarios

begin
-- criacao do sinal sign_A e sign_B, i.e., extensao do sinal A e B repetindo o bit mais significativo de duas formas alternativas
-- simbolo "&" representa a concatenacao dos bits
-- express�o "(7 downto 5 => B(9))" representa a repetic�o do bit de sinal B(4) nos bits 7, 6, e 5 de sign_B
sign_B <= B(4) & B(4) & B(4) & B(4 downto 0);
BL1 <=  B(4)& B(4) & B(4 downto 0)  & '0';



sign_A <= (7 downto 5 => A(4)) & A;
notA <= not(sign_A);
AL2  <= A(4) & A(4 downto 0) & '0' & '0';

som3mod <=  som3_S(7) & som3_S(7) & som3_S(7 downto 2);
som1mod <= som1_S(7) & som1_S(7) & som1_S(7) & som1_S(7 downto 3);--- porque é a dividir por 8





-- instancia dos SOMADORES (port map)
-- n�o modificar
SOM1: somador8 port map (
        P => som1_P, Q => som1_Q, cin => som1_cin,
        R => som1_S 
    );

SOM2: somador8 port map (
        P => som2_P, Q => som2_Q, cin => som2_cin,
        R => som2_S 
    );    

SOM3: somador8 port map (
        P => som3_P, Q => som3_Q, cin => som3_cin,
        R => som3_S 
    );

-- exemplo de um MUX2:1 de 8 bits controlado por I, com entradas sign_A e "00000000", 
-- ligado na entrada P do somador 1
-- DEVE SER MODIFICADO
    ---with I select
        ---som1_P <= sign_A   when '0',
             ---    "00000000" when others;

-- atribui��o das outras entradas dos somadores
-- DEVE SER MODIFICADO 
    som1_Q <= notA;
    som1_P <= sign_B;
    som1_cin <= '1';
    
    
    som2_P <= BL1;
    som2_Q <= sign_B;
    som2_cin <= '0';

    som3_P <= som2_S;
    som3_Q <= AL2;
    som3_cin <= '0';
    







-- atribui��o da sa�da do circuito aritm�tico projectado
-- DEVE SER MODIFICADO 
        with I select

    S <= som1mod when '0', 
        som3mod when others;
        
        
    
end Behavioral;

----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 11/14/2016 12:37:35 AM
-- Design Name: 
-- Module Name: tb_float_add - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity tb_phone_mgr is
--  Port ( );
end tb_phone_mgr;

architecture Behavioral of tb_phone_mgr is
  component phone_mgr 
    Port ( clk       : in STD_LOGIC;
           SUP       : in STD_LOGIC;
           ESQ       : in STD_LOGIC;
           DTO       : in STD_LOGIC;
           -- EstadoInit: in STD_LOGIC_VECTOR (7 downto 0);
           state_ME  : out STD_LOGIC_VECTOR (7 downto 0);
           saidas_ME : out STD_LOGIC_VECTOR (1 downto 0);
           timer_cnt : out STD_LOGIC_VECTOR (3 downto 0));
  end component;
  --inputs
    -- signal EstadoInit: STD_LOGIC_VECTOR (7 downto 0) := "00000001";
    signal clk : STD_LOGIC;
    signal ESQ, DTO, SUP : STD_LOGIC := '0';
    -- ouptuts 
    signal state_ME : STD_LOGIC_VECTOR(7 downto 0); 
    signal saidas_ME : STD_LOGIC_VECTOR (1 downto 0); 
    signal timer_cnt : STD_LOGIC_VECTOR (3 downto 0);
--    signal Dopen : STD_LOGIC;  
begin

-- instancia do componente (unidade de teste) 
    test_unit : phone_mgr port map (
     clk => clk, SUP => SUP,
     ESQ => ESQ, DTO => DTO, 
     state_ME => state_ME, -- EstadoInit => EstadoInit,
     saidas_ME => saidas_ME, timer_cnt => timer_cnt);
       
   -- process that will non-stop generate the clock signal
      -- '0' for 25ns, followed by '1' for another 25 ns (and so on...)
      process begin
          clk <= '0';
          wait for 25 ns;
          clk <= '1';
          wait for 25 ns;
      end process;
          
         -- process to test the circuit 
       process begin
           -- hold reset for the first 2 clock cycles 2x(2*25)
           SUP <= '1';
           wait for 2*60 ns; --desajusta com o flanco de clock
           SUP <= '0';
           wait for 100 ns;
           DTO <= '1';
           ESQ <= '1';
           wait for 50 ns;
           DTO <= '0';
           ESQ <= '0';
           wait for 10 ns;
           -- recebe chamada
           DTO <= '1'; 
           wait for 50 ns;
           DTO <= '0';
           -- espera por timeout e volta a espera
           wait for 500 ns; 
           -- recebe chamada
           DTO <= '1'; 
           wait for 50 ns;
           DTO <= '0';
           wait for 150 ns;
         -- vai para conversação
           DTO <= '1';
           wait for 50 ns;
           DTO <= '0';
           wait for 100 ns;
           DTO <= '1';
           ESQ <= '1';
           wait for 50 ns;
           DTO <= '0';
           ESQ <= '0';
           wait for 100 ns;
           -- vai para hold
           ESQ <= '1';
           wait for 50 ns;
           ESQ <= '0';
           wait for 100 ns;
           -- volta a conversação
           DTO <= '1';
           wait for 50 ns;
           DTO <= '0';
           wait for 100 ns;
           -- desliga e volta a espera
           DTO <= '1';
           wait for 50 ns;
           DTO <= '0';
           wait for 100 ns;
           -- vai para marcação
           ESQ <= '1';
           wait for 50 ns;
           ESQ <= '0';
           -- vai para chamar, faz timeout e volta para espera
           wait for 950 ns; 
           -- vai para marcação
           ESQ <= '1';
           wait for 50 ns;
           ESQ <= '0';
           -- vai para chamar, é atendido e vai para conversação
           wait for 450 ns; 
           DTO <= '1';
           wait for 50 ns;
           DTO <= '0';
           wait for 300 ns;
           -- volta a espera e termina
           DTO <= '1';
           wait for 50 ns;
           DTO <= '0';
           wait for 100 ns;
           
           
           DTO <= '1'; ---desligar a conversa---
           wait for 50 ns;
           DTO <= '0'; 
            wait for 100 ns;
            ESQ <= '1'; ---fazer marcacao ---
           wait for 50 ns;
           ESQ <= '0';
           
            wait for 1000 ns;
           
           
           wait; -- forever 
       end process;
 
end Behavioral;

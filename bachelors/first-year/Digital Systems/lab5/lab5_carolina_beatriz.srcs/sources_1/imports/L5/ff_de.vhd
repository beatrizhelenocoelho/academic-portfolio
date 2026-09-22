----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/25/2016 06:28:19 PM
-- Design Name: 
-- Module Name: ff_de - Behavioral
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

entity ff_de is
    Port ( din : in STD_LOGIC;
           clk : in STD_LOGIC;
           ce : in STD_LOGIC;
           reset : in STD_LOGIC;
           set : in STD_LOGIC;
           dout : out STD_LOGIC);
end ff_de;


-- Flip Flop D with asyncronous set and reset and clock enable changed to synchronous set and reset
architecture Behavioral of ff_de is

signal help, dout2 : STD_LOGIC;


begin
-- asynchronous set and reset
--   dout <= '0' when reset='1' else '1' when set='1' else
--            din when rising_edge(clk) and ce='1';

-- Synchronous set and reset

help <= (not reset) and (set or (ce and din) or ((not ce) and dout2));

dout2 <= help when rising_edge(clk);

dout <= dout2;

end Behavioral;


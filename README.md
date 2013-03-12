brew-PID-propane
================

An open source brewing controller to control brewing temperatures using an adjustable propane regulator (as on a turkey fryer) controlled by a servo, in turn controlled by an arduino using PID.
The goal is to have a fully-functional 2 or 3 pot brewing system that can use a RIMS, HERMS, direct fired, or insulated mash tun. 
This system should be automated, so that it shouldn't need human intervention.
 However, this is a propane fueled project, so you should not leave it unattended. 
**** Propane is highly combustable, poisonous, and can explode, killing you and burning down your house****
**** Do not leave any burner using this code unattended****

description and discussion of the project can be found at homebrewtalk - http://www.homebrewtalk.com/f235/any-interest-starting-new-open-source-automated-brewing-project-propane-380624/

I can be reached at rubinstein.james@gmail.com
good luck and happy homebrewing!
-JR

____________________________________________________________________________________________________________________________________________________________


V0.1 
This is the first version of the controller. It will control ONE burner.
*I'm using two Maxim (Dallas Semiconductor) Ds18b20 digital temperature sensors. 
*I'm also using one Sail winch servo to turn the regulator knob (this is the exact model http://www.ebay.com/itm/L-S785-Laser-Servo-Drum-Winch-Sail-boat-sailboat-winch-Same-as-Hitec-HS-785HB-/330833209709?pt=US_Radio_Control_Control_Line&hash=item4d0732296d)
*I'm using a Brinkmann turkey fryer with an adjustable regulator as my heat source
*This code is for Arduino UNO
*The sketch code itself is in the PID_temperature_to_servo_control_angle folder
*The libraries used are in the libraries folder
*Props to those who have gone before. The resources and inspirations I've used are in the resources-inspiration folder. The eventual goal is to get to something like HABS that uses the propane+PID heating system. 

Big to-dos:
*some safety system. The brinkmann fryer has a gas shutoff built in, so if the flame dies the gas shuts off. If you're not using a system with that, you may need to shutoff the gas manually in case there isn't enough gas to support a flame, but still enough to build up causing your untimely demise.
The brinkmann fryer doesn't have electric ignition, so it requires manually re-lighting when the gas is turned back up. 
the best solution would probably be a standing pilot ignition system. 

*The PID settings need to be tuned. I tried running an auto tune with no success. I will have to manually tune when I have some time.

*Adding timers, displays, etc.



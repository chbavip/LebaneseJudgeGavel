# LebaneseJudgeGavel
Lebanese Judge Gavel Project - OpenGL

1- INTRODUCTION:
The Judiciary plays a crucial role in achieving Justice, security and stability within society and state.
The goal of this project is to develop a C++ application with Open Graphics Library (OpenGL), that simulates symbolically the important role of Judges in the Lebanese state.
The project shows in the upper area of the screen the Lebanese Flag which represents the Lebanese state, and beneath of it a Judge’s block surface which represents a Lebanese court that is a source of issued judicial sentences.
The Gavel of the Judge represents the judge’s authority to issue judicial decisions & sentences, which must be in continuous action to fulfill justice, while screen edges represent the rule of law that limits the Judge’s power. The speed of the Gavel can be increased: this represents speeding up justice delivery. Also, the Gavel can be halted: this represents issuing a judicial decision or sentence; the block upper surface changes instantly to red, which represents that it is being strike by the Gavel.
Gavel returns back to its movement; the block upper surface takes back its original color.
The rotation of a 3D Lebanese flag, with the following text “Lebanese Judicial System”: this represent that No matter what happens in Lebanon, the Lebanese Judicial System will remain standing up to do its honorable legitimate tasks.
The application implements the following functionalities:
-	Draw 2D & 3D objects (together in same space) 
-	Two-Dimensional Geometric Transformation (Rotation, Tranlation)
- Three-Dimensional Geometric Transformation (Rotation, Tranlation)
-	Display OpenGL Characters
-	Display-Window Reshape
-	Mouse Interaction
-	Keyboard Interaction
-	Computer Animation (2D, 3D)
The remaining of this report is organized as follows; section 2 analyzes the need for this project and specifies the business rules. Section 3 elaborates on the project implementation while section 4 concludes this report.
 
2- BUSINESS RULES:
This section discusses the business rules specifying the project’s aims. The business requirements are relatively straightforward and can be divided into 7 requirements:
1.	Draw the main objects (“Lebanese Flag” (2D & 3D), “Judge’s Gavel”, “Judge’s Block”).
2.	Fix the following objects on black background:
-	“Lebanese Flag (2D)” on the top-left side of the screen.
-	“Judge’s Block” on the bottom-left side of the screen.
-	“Lebanese Flag (3D)” on the rigth side of the screen.
3.	Auto-Animate in 2D the Judge Gavel, in such a way: 
-	to make it move continuously in all directions, on specific speed.
-	to make it bounce on the 4 edges of the screen.
4.	Auto-Animate in 3D the Lebanese Flag (3D), in such a way to make it spin continuously in all directions, on low speed, with a relevant text.
5.	Allow user to control the movement of the gavel: 
-	by increasing/decreasing the speed,
-	by stoping/starting the movement itself.
6.	Allow user to get notified by a text & by an object color change, when gavel is stopped.
7.	Allow all objects to be reshaped without being distorted, either when changing the size of display window, or when draging the display window to another screen location.

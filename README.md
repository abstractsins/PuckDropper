# Puck Dropper

## `C++` 2D Physics game using `SFML 3.0`

*This is my first C++ project. I built it to show what I can learn and build from scratch.*

Check out a [video](https://www.youtube.com/watch?v=R15gpBx2NWo) of the game in action.

![Title Screen of the game](https://i.imgur.com/cF5T2LK.png)

**ABOUT**

* Guide a puck into slots of different score values while building as few segments as possible.

* The idea formed out of a Plinko-style game from The Price is Right. In my version I decided the user would have to lay out a sort of path for the puck, with the threat of the puck breaking if it falls too far on its own. 

* While building the game I had a lot of fun playing around with the simulator, so I decided to make a "free mode" where the user can just have fun. 

* In scoring mode there is a time bonus for getting through the course quickly. There are also penalties for every collision, and the amount of connections made for guiding the puck. There is a high score recording mechanism.

**CHALLENGES**

* I had to implement a custom Button class to deal with UI elements since SFML does not have native button elements.

* I had to overcome some issues and challenges like: making the whole puck part of the collision, not just the center of it. I had to make certain decisions like: should the user only be able to connect neighboring dots? 

**FUTURE VERSIONS?**

* If I were to work on future versions it may contain things like, more levels, initial puck positioning, more physics customization, and better graphics.

**HOW TO INSTALL**

* The release contains a folder with everything you need. Unzip it wherever you want and run `Game.exe`.

**CREDITS**

* Music by twistedLoop
* sounds from Kenney.nl
* Stock photos from unsplash.com

Sliding Tiles Game
---

This is a simple game where a picture is broken up into a number of tiles.  The
tiles are then randomly shuffled around.  The main goal of the player is to 
slide each tile into the empty slot to re-arrange the picture.

If the executable is run without any flags it will look for a 'default.jpg'
in the same directory as the program and use that as the image.  The skill
level will be set to Easy.

Usage involves two flags:

  * --image (or -i) [filename] to specify a different image.
  * --skill (or -s) [e|m|h] to change the skill.  e = easy, m = medium and 
    h = hard.

The game keeps track of how long its been played and how many tile moves have
occured.  When the picture is completed the countdown will stop and no tiles
will be moveable.  Note that the empty spot will always be the upper left-hand
corner of the board.
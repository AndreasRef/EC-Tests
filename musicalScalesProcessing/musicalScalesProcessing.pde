//Quick test for making a flexible note and scale music system
String[] notes = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

int[] majorScale = {0, 2, 4, 5, 7, 9, 11};
int[] minorScale = {0, 2, 3, 5, 7, 8, 10}; 

void setup() {
printNote(201, 2, "minor");
printNote(200, 2, "major");
}

void printNote(int startingNote, int step, String scale) {
  
  if (scale == "major" ) {
  println(notes[(startingNote + majorScale[step % majorScale.length]) % notes.length] + " " + (floor((startingNote +  majorScale[step]) / notes.length)-1) + " scale: " + scale);
  } else if (scale == "minor" ) {
  println(notes[(startingNote + minorScale[step % minorScale.length]) % notes.length] + " " + (floor((startingNote +  minorScale[step]) / notes.length)-1) + " scale: " + scale);
  }
}
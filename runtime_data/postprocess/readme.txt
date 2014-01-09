Each line is a possible lp pattern organized by region/state and then likelihood.

The parser goes through each line and tries to match
@ = any letter
# = any number
? = a skip position (can be anything, but remove it if encountered)
[A-FGZ] is just a single char position with specific letter requirements.  In this example, the regex defines characters ABCDEFGZ

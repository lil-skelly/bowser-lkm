savedcmd_/home/skelly/projects/bowser/bowser.mod := printf '%s\n'   bowser.o | awk '!x[$$0]++ { print("/home/skelly/projects/bowser/"$$0) }' > /home/skelly/projects/bowser/bowser.mod

rm pack.zip
find . | egrep "\.(cpp|h)$" | zip -@ pack.zip

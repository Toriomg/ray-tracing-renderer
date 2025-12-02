#import "@preview/lilaq:0.5.0" as lq

// 1. Read the index using a root-absolute path (starts with /)
#let file-list = csv("/graphs/index.csv").flatten()

#for filename in file-list {
    
    // Clean up whitespace and skip the index file itself
    let fname = filename.trim()
    if fname == "index.csv" or fname == "" { continue }

    // 2. Construct the path starting with "/" to indicate Project Root
    let path = "/" + fname
    
    // Optional: Header for the table
    block(breakable: false)[
        #strong[File: #fname]
        #v(0.5em)

        #table(
            columns: 2,
            // 3. Your exact loop syntax
            ..for (.., NIA, nota) in csv(path) {
                (NIA, nota)
            }
        )
    ]
    v(2em)
}
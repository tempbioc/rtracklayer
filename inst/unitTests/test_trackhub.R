test_trackhub <- function() {
    test_trackhub_path <- system.file("tests", "trackhub", package = "rtracklayer")
    th <- TrackHub(test_trackhub_path)

    correct_uri <- file.path("file://", test_trackhub_path)
    correct_genome <- "hg19"
    correct_length <- as.integer(1)
    correct_hub <- "test_hub"
    correct_shortLabel <- "test_hub"
    correct_longLabel <- "test_hub"
    correct_genomesFile <- "genomes.txt"
    correct_email <- "user@domain.com"
    correct_descriptionUrl <- "http://www.somedomain.com/articles/h19"

    ## TEST: uri
    checkIdentical(uri(th), correct_uri)

    ## TEST: genome
    checkIdentical(genome(th), correct_genome)

    ## TEST: length
    checkIdentical(length(th), correct_length)

    # TEST: hub
    checkIdentical(hub(th), correct_hub)

    # TEST: shortLabel
    checkIdentical(shortLabel(th), correct_shortLabel)

    # TEST: longLabel
    checkIdentical(longLabel(th), correct_longLabel)

    # TEST: genomeFile
    checkIdentical(genomeFile(th), correct_genomesFile)

    # TEST: email
    checkIdentical(email(th), correct_email)

    # TEST: descriptionUrl
    checkIdentical(descriptionUrl(th), correct_descriptionUrl)

    # TEST: hub<-
    new_hub <- "new_hub"
    hub(th) <- new_hub
    checkIdentical(hub(th), new_hub)
    hub(th) <- correct_hub

    # TEST: shortLabel<-
    new_shortLabel <- "new_hub"
    shortLabel(th) <- new_shortLabel
    checkIdentical(shortLabel(th), new_shortLabel)
    shortLabel(th) <- correct_shortLabel

    # TEST: longLabel<-
    new_longLabel <- "new_hub"
    longLabel(th) <- new_longLabel
    checkIdentical(longLabel(th), new_longLabel)
    longLabel(th) <- correct_longLabel

    # TEST: genomeFile<-
    new_genomeFile <- "newfile.txt"
    genomeFile(th) <- new_genomeFile
    checkIdentical(genomeFile(th), new_genomeFile)
    genomeFile(th) <- correct_genomesFile

    # TEST: email<-
    new_email <- "new@domail.com"
    email(th) <- new_email
    checkIdentical(email(th), new_email)
    email(th) <- correct_email

    # TEST: descriptionUrl<-
    new_descriptionUrl <- "http://newdomail.com/articles/hg19"
    descriptionUrl(th) <- new_descriptionUrl
    checkIdentical(descriptionUrl(th), new_descriptionUrl)
    descriptionUrl(th) <- correct_descriptionUrl
}
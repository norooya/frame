pipeline {
    agent any
    stages {
        stage('echo') {
            steps {
                sh "echo ${params.GIT_TAG} > params.txt"
            }
        }
    }
}

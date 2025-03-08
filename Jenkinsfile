// https://www.jenkins.io/doc/book/pipeline/jenkinsfile/
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

/* Groovy의 다중줄 문자열 문법(''')을 사용합니다
pipeline {
    agent any
    stages {
        stage('Example Stage') {
            steps {
                script {
                    // 여러 줄 명령어 실행을 위해 ''' 사용
                    sh '''
                        echo "Hello, World"
                        ls -l
                        # 다른 명령어 추가 가능
                    '''
                }
            }
        }
    }
}
*/

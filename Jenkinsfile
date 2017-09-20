pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        sh '''./configure
make'''
      }
    }
  }
}
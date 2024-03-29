![Logo](_media/chtuner_logo.png)</br></br></br>

<img align="left" width="60%" src="_media/screen_01.png"/>
<img align="right" width="40%" src="_media/screen_02.png"/>

The aim of the project is the production of an application that a musician can use for tune his instruments. 

This application use a Pitch Detection Algorithm (MPM algorithm [See references](#references)), for identify the pitch of a sound recorded by the microphone. This implementation runs in real-time recording audio signal for a small delta of time and applaying the algorithm.

The User-Interface show to the user the nearest note calculated starting from the pitch identified.

</br></br></br>

The application provides a dialog box for set the user preferences, such as:
   * the frequency of A4;
   * the input device;
   * the notation tu use for display the note.
   
</br></br>

## Getting Started

A compiled version of the software can be installed downloading downloading the [setup](setup.exe) executable.

## Deployment

## Built With 
The following libraries are used for the development of the application:

* [Portaudio](http://www.portaudio.com) a free, cross-platform, open-source, audio I/O library;
* [FFTW3](http://www.fftw.org/) a C subroutine library for computing the discrete Fourier transform.

### Authors

* **Mario Randazzo** - *Initial work* - [MR6996](https://github.com/MR6996)

### License

This project is licensed under the GNU GPL License - see the [LICENSE](LICENSE) file for details

## References
[1] **P. McLeod and G. Wyvill. A smarter way to find pitch.** In Proceedings of the International Computer Music Conference (ICMC’05), 2005.

Copyright © 2018-2022 Mario Randazzo

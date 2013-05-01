// Copyright 2013 Erik Gilling <konkers@konkers.net>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the Licene.

#ifndef __TEMPSENSOR_HPP__
#define __TEMPSENSOR_HPP__

#include <string>

class TempSensor {
public:
	virtual float getTemp(void) = 0;
	virtual const std::string getName(void) = 0;
};

#endif /* __TEMPSENSOR_HPP__ */
